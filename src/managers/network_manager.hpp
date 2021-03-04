#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include <map>
#include <mutex>
#include <string>
#include <thread>

#include <concurrentqueue/concurrentqueue.h>
#include <entt/entity/registry.hpp>

#define ASIO_STANDALONE
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

class NetworkManager
{

    public:

        typedef websocketpp::server<websocketpp::config::asio> ServerType;

        NetworkManager(entt::registry& _Reg) : Reg_(_Reg) {}

        bool isRunning() const {return IsRunning_;}

        bool init(moodycamel::ConcurrentQueue<std::string>* const _InputQueue,
                  moodycamel::ConcurrentQueue<std::string>* const _OutputQueue,
                  int _Port);
        bool stop();

    private:

        void onMessage(websocketpp::connection_hdl, ServerType::message_ptr _Msg);
        bool onValidate(websocketpp::connection_hdl);
        void send();

        entt::registry& Reg_;

        moodycamel::ConcurrentQueue<std::string>* InputQueue_;
        moodycamel::ConcurrentQueue<std::string>* OutputQueue_;

        ServerType Server_;
        std::map<std::string, websocketpp::connection_hdl> Connections_;
        std::mutex ConnectionsLock_;

        std::thread ThreadSender_;
        std::thread ThreadServer_;

        bool IsRunning_{true};
};

#endif // NETWORK_MANAGER_HPP
