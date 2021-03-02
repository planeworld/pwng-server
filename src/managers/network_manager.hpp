#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

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

        void init(moodycamel::ConcurrentQueue<std::string>* const _InputQueue,
                  moodycamel::ConcurrentQueue<std::string>* const _OutputQueue,
                  int _Port);
        void onMessage(websocketpp::connection_hdl, ServerType::message_ptr _Msg);

    private:

        void send()
        {
            while (true)
            {
                std::cout << "Sending messages" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        moodycamel::ConcurrentQueue<std::string>* InputQueue_;
        moodycamel::ConcurrentQueue<std::string>* OutputQueue_;

        ServerType Server_;
        websocketpp::lib::shared_ptr<websocketpp::lib::thread> Thread_;
        std::thread ThreadSender_;
};

#endif // NETWORK_MANAGER_HPP
