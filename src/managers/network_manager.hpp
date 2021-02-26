#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include <concurrentqueue/concurrentqueue.h>

#define ASIO_STANDALONE
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

class NetworkManager
{

    public:

        typedef websocketpp::server<websocketpp::config::asio> ServerType;

        void init(moodycamel::ConcurrentQueue<std::string>* const _InputQueue,
                  moodycamel::ConcurrentQueue<std::string>* const _OutputQueue);
        void onMessage(websocketpp::connection_hdl, ServerType::message_ptr _Msg);

    private:

        moodycamel::ConcurrentQueue<std::string>* InputQueue_;
        moodycamel::ConcurrentQueue<std::string>* OutputQueue_;

        ServerType Server_;
        websocketpp::lib::shared_ptr<websocketpp::lib::thread> Thread_;
};

#endif // NETWORK_MANAGER_HPP
