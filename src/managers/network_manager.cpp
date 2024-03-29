#include "network_manager.hpp"

#include "message_handler.hpp"
#include "network_message_broker.hpp"
#include "timer.hpp"

bool NetworkManager::init(moodycamel::ConcurrentQueue<NetworkMessageParsed>* const _QueueNetIn,
                          moodycamel::ConcurrentQueue<NetworkMessage>* const _InputQueue,
                          moodycamel::ConcurrentQueue<NetworkMessage>* const _OutputQueue,
                          int _Port)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    Messages.report("net", "Initialising Network Manager", MessageHandler::INFO);

    QueueNetIn_ = _QueueNetIn;
    InputQueue_ = _InputQueue;
    OutputQueue_ = _OutputQueue;

    Server_.set_access_channels(websocketpp::log::alevel::all);
    Server_.clear_access_channels(websocketpp::log::alevel::frame_header);
    Server_.clear_access_channels(websocketpp::log::alevel::frame_payload);
    Server_.set_error_channels(websocketpp::log::elevel::all);
    Server_.get_elog().set_ostream(&ErrorStream_);
    Server_.get_alog().set_ostream(&MessageStream_);
    Server_.set_close_handler(std::bind(&NetworkManager::onClose, this,
                              std::placeholders::_1));
    Server_.set_message_handler(std::bind(&NetworkManager::onMessage, this,
                                std::placeholders::_1, std::placeholders::_2));
    Server_.set_validate_handler(std::bind(&NetworkManager::onValidate, this,
                                 std::placeholders::_1));

    Server_.init_asio();
    try
    {
        Server_.listen(_Port);
        Messages.report("net", "Server listening on port " + std::to_string(_Port), MessageHandler::INFO);
    }
    catch (const websocketpp::exception& e)
    {
        Messages.report("net", "Couldn't listen to port " + std::to_string(_Port));
        return false;
    }

    websocketpp::lib::error_code ErrorCode;
    Server_.start_accept(ErrorCode);
    if (ErrorCode)
    {
        Messages.report("net", "Couldn't start server: " + ErrorCode.message());
        std::cerr << ErrorCode.message() << std::endl;
    }

    ThreadServer_ = std::thread(std::bind(&ServerType::run, &Server_));
    ThreadSender_ = std::thread(&NetworkManager::run, this);

    return true;
}

void NetworkManager::onClose(websocketpp::connection_hdl _Connection)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    std::lock_guard<std::mutex> Lock(ConnectionsLock_);

    auto ID = ConHdlToID_[_Connection];
    Connections_.erase(_Connection);
    ConIDToHdl_.erase(ID);
    ConHdlToID_.erase(_Connection);
    Reg_.destroy(entt::entity(ID));

    Messages.report("net", "Connection to client ID "+std::to_string(entt::to_integral(ID)) + " closed ("+std::to_string(Connections_.size())+ " open connection(s)).", MessageHandler::INFO);
}

void NetworkManager::onMessage(websocketpp::connection_hdl _Connection, ServerType::message_ptr _Msg)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    DBLK(Messages.report("net", "Enqueueing incoming message from ID: "
                         + std::to_string(entt::to_integral(ConHdlToID_[_Connection]))+"\n"
                         + _Msg->get_payload(), MessageHandler::DEBUG_L3);)

    InputQueue_->enqueue({ConHdlToID_[_Connection], _Msg->get_payload()});
}

bool NetworkManager::onValidate(websocketpp::connection_hdl _Connection)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    websocketpp::server<websocketpp::config::asio>::connection_ptr
        Connection = Server_.get_con_from_hdl(_Connection);
    websocketpp::uri_ptr Uri = Connection->get_uri();

    DBLK(Messages.report("net", "Query string: " + Uri->get_query(), MessageHandler::DEBUG_L1);)

    std::lock_guard<std::mutex> Lock(ConnectionsLock_);

    // Store connection data and a unique id for further
    // assignment of message queries
    Connections_.insert(_Connection);
    auto e = Reg_.create();
    ConIDToHdl_[e] = _Connection;
    ConHdlToID_[_Connection] = e;

    Messages.report("net", "Connection to client ID " + std::to_string(entt::to_integral(e)) + " validated ("+std::to_string(Connections_.size())+ " open connection(s)).", MessageHandler::INFO);

    return true;
}

void NetworkManager::run()
{
    auto& Messages = Reg_.ctx<MessageHandler>();
    auto& Broker = Reg_.ctx<NetworkMessageBroker>();
    Timer NetworkTimer;

    Messages.report("net", "Network Manager running", MessageHandler::INFO);

    while (IsRunning_)
    {
        NetworkTimer.start();

        // Check, if there are any errors or messages from
        // websocketpp.
        // Since output is transferred to a (string-)stream,
        // it's content is frequently checked
        if (!ErrorStream_.str().empty())
        {
            // Extract line by line in case of multiple messages
            std::string Line;
            while (std::getline(ErrorStream_, Line, '\n'))
            {
                Messages.report("net", "Websocket++: "+Line);
            }
            ErrorStream_.str({});
        }
        DBLK(
            if (!MessageStream_.str().empty())
            {
                std::string Line;
                while(std::getline(MessageStream_, Line, '\n'))
                {
                    Messages.report("net", "Websocket++: "+Line, MessageHandler::DEBUG_L1);
                }
                MessageStream_.str({});
            }
        )

        NetworkMessage Message;
        while (OutputQueue_->try_dequeue(Message))
        {
            auto Con = ConIDToHdl_[Message.ClientID];
            websocketpp::lib::error_code ErrorCode;
            Server_.send(Con, Message.Payload, websocketpp::frame::opcode::text, ErrorCode);
            if (ErrorCode)
            {
                Messages.report("net", "Sending failed: " + ErrorCode.message());
            }

        }

        NetworkMessageParsed d;

        while (QueueNetIn_->try_dequeue(d))
        {
            Broker.executeNet(d);
        }

        NetworkTimer.stop();
        if (NetworkingStepSize_ - NetworkTimer.elapsed_ms() > 0.0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(NetworkingStepSize_ - int(NetworkTimer.elapsed_ms())));
        }
        else
        {
            Messages.report("net", "Thread processing exceeds step time ("
                            + std::to_string(NetworkTimer.elapsed_ms())+"/"
                            + std::to_string(NetworkingStepSize_)+")ms",
                            MessageHandler::WARNING);
        }
    }
    DBLK(Messages.report("net", "Sender thread stopped successfully", MessageHandler::DEBUG_L1);)
}

bool NetworkManager::stop()
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    Messages.report("net", "Stopping server", MessageHandler::INFO);

    websocketpp::lib::error_code ErrorCode;
    Server_.stop_listening(ErrorCode);
    if (ErrorCode)
    {
        Messages.report("net", "Stopping server failed: " + ErrorCode.message());
        return false;
    }

    std::lock_guard<std::mutex> Lock(ConnectionsLock_);
    for (auto Con : Connections_)
    {
        websocketpp::lib::error_code ErrorCode;
        Server_.close(Con, websocketpp::close::status::normal,
                      "Server shutting down, closing connection.", ErrorCode);
        if (ErrorCode)
        {
            Messages.report("net", "Closing connection failed: " + ErrorCode.message());
        }
    }

    Server_.stop();
    IsRunning_ = false;

    ThreadServer_.join();
    ThreadSender_.join();

    Messages.report("net", "Server stopped", MessageHandler::INFO);

    return true;
}
