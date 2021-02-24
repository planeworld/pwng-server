#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#define ASIO_STANDALONE

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

class NetworkManager
{

    public:

        typedef websocketpp::server<websocketpp::config::asio> ServerType;

        NetworkManager()
        {
            // Server_.set_message_handler(&NetworkManager::on_message);
        }

        void on_message(websocketpp::connection_hdl, ServerType::message_ptr msg)
        {
            std::cout << msg->get_payload() << std::endl;
        }


    private:

        asio::io_service IOService_;
        ServerType Server_;

};

#endif // NETWORK_MANAGER_HPP
