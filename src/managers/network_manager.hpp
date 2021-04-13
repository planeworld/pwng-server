#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>

#include <concurrentqueue/concurrentqueue.h>
#include <entt/entity/registry.hpp>

#define ASIO_STANDALONE
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "network_message.hpp"

class NetworkManager
{

    public:

        typedef websocketpp::server<websocketpp::config::asio> ServerType;

        NetworkManager(entt::registry& _Reg) : Reg_(_Reg) {}

        bool isRunning() const {return IsRunning_;}

        bool init(moodycamel::ConcurrentQueue<NetworkMessage>* const _InputQueue,
                  moodycamel::ConcurrentQueue<NetworkMessage>* const _OutputQueue,
                  int _Port);
        bool stop();

    private:

        void onClose(websocketpp::connection_hdl);
        void onMessage(websocketpp::connection_hdl, ServerType::message_ptr _Msg);
        bool onValidate(websocketpp::connection_hdl);
        void run();

        entt::registry& Reg_;

        std::stringstream ErrorStream_;
        std::stringstream MessageStream_;

        moodycamel::ConcurrentQueue<NetworkMessage>* InputQueue_;
        moodycamel::ConcurrentQueue<NetworkMessage>* OutputQueue_;

        std::uint32_t NetworkingStepSize_{10};

        ServerType Server_;

        //--- Connections ---//
        std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> Connections_;
        std::mutex ConnectionsLock_;

        std::map<websocketpp::connection_hdl, ConIDType, std::owner_less<websocketpp::connection_hdl>> ConHdlToID_;
        std::map<ConIDType, websocketpp::connection_hdl> ConIDToHdl_;

        //--- Threads ---//
        std::thread ThreadSender_;
        std::thread ThreadServer_;

        bool IsRunning_{true};

        // No need to worry about a specialized id class that frees up unused
        // ids. With uint64 the server could run 585 billion years with one
        // connection per second without overflowing
        static ConIDType ConnectionIDCounter_;
};

#endif // NETWORK_MANAGER_HPP
