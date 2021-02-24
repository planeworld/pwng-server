#include <iostream>

#include <entt/entity/registry.hpp>

#include "network_manager.hpp"


void on_message(websocketpp::connection_hdl, NetworkManager::ServerType::message_ptr msg)
{
    std::cout << msg->get_payload() << std::endl;
}

int main()
{
    entt::registry Reg;
    NetworkManager NetMan;

    NetworkManager::ServerType Server;

    // Reg.set<NetworkManager>(NetMserver Server;

    Server.set_message_handler(&on_message);
    Server.set_access_channels(websocketpp::log::alevel::all);
    Server.set_error_channels(websocketpp::log::elevel::all);

    Server.init_asio();
    Server.listen(9002);
    websocketpp::lib::error_code ec;
    Server.start_accept(ec); // omit error handling to keep example consise

    Server.run();

    return EXIT_SUCCESS;
}
