#include "network_manager.hpp"

#include <nlohmann/json.hpp>

NetworkManager::~NetworkManager()
{
    this->stop();
}

bool NetworkManager::init(moodycamel::ConcurrentQueue<std::string>* const _InputQueue,
                          moodycamel::ConcurrentQueue<std::string>* const _OutputQueue,
                          int _Port)
{
    InputQueue_ = _InputQueue;
    OutputQueue_ = _OutputQueue;

    Server_.clear_access_channels(websocketpp::log::alevel::all);
    Server_.clear_error_channels(websocketpp::log::elevel::all);

    Server_.set_message_handler(std::bind(&NetworkManager::onMessage, this,
                                std::placeholders::_1, std::placeholders::_2));
    Server_.set_validate_handler(std::bind(&NetworkManager::onValidate, this,
                                 std::placeholders::_1));

    Server_.init_asio();
    try
    {
        Server_.listen(_Port);
    }
    catch (const websocketpp::exception& e)
    {
        std::cerr << "Couldn't listen to port " << _Port << std::endl;
        return false;
    }

    websocketpp::lib::error_code ErrorCode;
    Server_.start_accept(ErrorCode);
    if (ErrorCode)
    {
        std::cerr << ErrorCode.message() << std::endl;
    }

    ThreadServer_ = std::thread(std::bind(&ServerType::run, &Server_));
    ThreadSender_ = std::thread(&NetworkManager::send, this);

    return true;
}

void NetworkManager::onMessage(websocketpp::connection_hdl _Connection, ServerType::message_ptr _Msg)
{
    InputQueue_->enqueue(_Msg->get_payload());
}

bool NetworkManager::onValidate(websocketpp::connection_hdl _Connection)
{
    websocketpp::server<websocketpp::config::asio>::connection_ptr
        Connection = Server_.get_con_from_hdl(_Connection);
    websocketpp::uri_ptr Uri = Connection->get_uri();

    std::string Query = Uri->get_query();

    std::cout << "Query: " << Query << std::endl;
    // if (!query.empty()) {
    //     // Split the query parameter string here, if desired.
    //     // We assume we extracted a string called 'id' here.
    // }
    // else {
    //     // Reject if no query parameter provided, for example.
    //     return false;
    // }
    std::string ID = "1";
    std::cout << "Connection validated" << std::endl;
    std::lock_guard<std::mutex> Lock(ConnectionsLock_);
    Connections_.insert({ID, _Connection});

    return true;
}

void NetworkManager::send()
{
    while (true)
    {
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
                    std::cerr << ErrorCode.message() << std::endl;
                }
            }
            else
            {
                std::cout << "Socket unknown, message dropped" << std::endl;
            }

            // std::cout << Message << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool NetworkManager::stop()
{
    websocketpp::lib::error_code ErrorCode;
    Server_.stop_listening(ErrorCode);
    if (ErrorCode)
    {
        std::cerr << ErrorCode.message() << std::endl;
        return false;
    }

    // Close all existing websocket connections.
    // std::string data = "Terminating connection...";
    // map<string, connection_hdl>::iterator it;
    // for (it = websockets.begin(); it != websockets.end(); ++it) {
    //     websocketpp::lib::error_code ec;
    //     server.close(it->second, websocketpp::close::status::normal, data, ec); // send text message.
    //     if (ec) { // we got an error
    //         // Error closing websocket. Log reason using ec.message().
    //     }
    // }

    Server_.stop();
    std::cout << "Server stopped" << std::endl;
    return true;
}
