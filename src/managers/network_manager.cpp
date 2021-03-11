#include "network_manager.hpp"

#include "error_handler.hpp"
#include "message_handler.hpp"

bool NetworkManager::init(moodycamel::ConcurrentQueue<std::string>* const _InputQueue,
                          moodycamel::ConcurrentQueue<std::string>* const _OutputQueue,
                          int _Port)
{
    auto& Errors = Reg_.ctx<ErrorHandler>();
    auto& Messages = Reg_.ctx<MessageHandler>();

    Messages.report("NET", "Initialising Network Manager");

    InputQueue_ = _InputQueue;
    OutputQueue_ = _OutputQueue;

    Server_.clear_access_channels(websocketpp::log::alevel::all);
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
        Messages.report("NET", "Server listening on port " + std::to_string(_Port));
    }
    catch (const websocketpp::exception& e)
    {
        Errors.report("NET", "Couldn't listen to port " + std::to_string(_Port));
        return false;
    }

    websocketpp::lib::error_code ErrorCode;
    Server_.start_accept(ErrorCode);
    if (ErrorCode)
    {
        Errors.report("NET", "Couldn't start server: " + ErrorCode.message());
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
    Connections_.erase("1");

    Messages.report("NET", "Connection to client closed");
}

void NetworkManager::onMessage(websocketpp::connection_hdl _Connection, ServerType::message_ptr _Msg)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    DBLK(Messages.report("NET", "Incoming message enqueued", MessageHandler::DEBUG_L2);)
    DBLK(Messages.report("NET", "Content: " + _Msg->get_payload(), MessageHandler::DEBUG_L3);)
    InputQueue_->enqueue(_Msg->get_payload());
}

bool NetworkManager::onValidate(websocketpp::connection_hdl _Connection)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    websocketpp::server<websocketpp::config::asio>::connection_ptr
        Connection = Server_.get_con_from_hdl(_Connection);
    websocketpp::uri_ptr Uri = Connection->get_uri();

    DBLK(Messages.report("NET", "Query string: " + Uri->get_query(), MessageHandler::DEBUG_L1);)

    std::string ID = "1";
    Messages.report("NET", "Connection validated");
    std::lock_guard<std::mutex> Lock(ConnectionsLock_);
    Connections_.insert({ID, _Connection});

    return true;
}

void NetworkManager::run()
{
    auto& Errors = Reg_.ctx<ErrorHandler>();
    auto& Messages = Reg_.ctx<MessageHandler>();

    Messages.report("NET", "Network Manager running");

    while (IsRunning_)
    {
        // Check, if there are any errors or messages from
        // websocketpp.
        // Since output is transferred to a (string-)stream,
        // it's content is frequently checked
        std::string ErrorStreamContent{ErrorStream_.str()};
        if (!ErrorStreamContent.empty())
        {
            // First, remove the carriage return, since report
            // includes a std::endl. Otherwise, it would result
            // in an empty line
            ErrorStreamContent.pop_back();
            Errors.report("NET", "Websocket++: "+ErrorStreamContent);
            ErrorStream_.str({});
        }
        std::string MessageStreamContent{MessageStream_.str()};
        if (!MessageStreamContent.empty())
        {
            MessageStreamContent.pop_back();
            Messages.report("NET", "Websocket++: "+MessageStreamContent);
            MessageStream_.str({});
        }


        std::string Message;
        while (OutputQueue_->try_dequeue(Message))
        {
            std::string ID = "1";

            auto it = Connections_.find(ID);
            if (it != Connections_.end())
            {
                auto Connection = it->second;
                websocketpp::lib::error_code ErrorCode;
                Server_.send(Connection, Message, websocketpp::frame::opcode::text, ErrorCode);
                if (ErrorCode)
                {
                    Errors.report("NET", "Sending failed: " + ErrorCode.message());
                }
            }
            else
            {
                // std::cout << "Socket unknown, message dropped" << std::endl;
            }

            // std::cout << Message << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    DBLK(Messages.report("NET", "Sender thread stopped successfully", MessageHandler::DEBUG_L1);)
}

bool NetworkManager::stop()
{
    auto& Errors = Reg_.ctx<ErrorHandler>();
    auto& Messages = Reg_.ctx<MessageHandler>();

    Messages.report("NET", "Stopping server");

    websocketpp::lib::error_code ErrorCode;
    Server_.stop_listening(ErrorCode);
    if (ErrorCode)
    {
        Errors.report("NET", "Stopping server failed: " + ErrorCode.message());
        return false;
    }

    std::map<std::string, websocketpp::connection_hdl>::iterator it;
    for (it = Connections_.begin(); it != Connections_.end(); ++it)
    {
        websocketpp::lib::error_code ErrorCode;
        Server_.close(it->second, websocketpp::close::status::normal,
                      "Server shutting down, closing connection.", ErrorCode);
        if (ErrorCode)
        {
            Errors.report("NET", "Closing connection failed: " + ErrorCode.message());
        }
    }

    Server_.stop();
    IsRunning_ = false;

    ThreadServer_.join();
    ThreadSender_.join();

    Messages.report("NET", "Server stopped");

    return true;
}
