#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#define ASIO_STANDALONE

#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

class NetworkManager
{

    public:

        typedef websocketpp::server<websocketpp::config::asio> ServerType;

        void init();
        void onMessage(websocketpp::connection_hdl, ServerType::message_ptr _Msg);

    private:

        ServerType Server_;
        websocketpp::lib::shared_ptr<websocketpp::lib::thread> Thread_;
};

#endif // NETWORK_MANAGER_HPP
