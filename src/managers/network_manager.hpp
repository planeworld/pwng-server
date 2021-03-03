#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include <map>
#include <mutex>
#include <string>
#include <thread>

#include <concurrentqueue/concurrentqueue.h>

#define ASIO_STANDALONE
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

class NetworkManager
{

    public:

        typedef websocketpp::server<websocketpp::config::asio> ServerType;

        ~NetworkManager();

        bool init(moodycamel::ConcurrentQueue<std::string>* const _InputQueue,
                  moodycamel::ConcurrentQueue<std::string>* const _OutputQueue,
                  int _Port);
        bool stop();

    private:

        void onMessage(websocketpp::connection_hdl, ServerType::message_ptr _Msg);
        bool onValidate(websocketpp::connection_hdl);
        void send();

        moodycamel::ConcurrentQueue<std::string>* InputQueue_;
        moodycamel::ConcurrentQueue<std::string>* OutputQueue_;

        ServerType Server_;
        std::map<std::string, websocketpp::connection_hdl> Connections_;
        std::mutex ConnectionsLock_;

        std::thread ThreadSender_;
        std::thread ThreadServer_;
};

#endif // NETWORK_MANAGER_HPP
