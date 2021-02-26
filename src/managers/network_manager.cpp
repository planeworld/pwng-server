#include "network_manager.hpp"

#include <nlohmann/json.hpp>

// For convenience
using json = nlohmann::json;

void NetworkManager::init()
{
    Server_.clear_access_channels(websocketpp::log::alevel::all);
    Server_.clear_error_channels(websocketpp::log::elevel::all);

    Server_.set_message_handler(std::bind(&NetworkManager::onMessage, this,
                                            std::placeholders::_1, std::placeholders::_2));

    Server_.init_asio();
    Server_.listen(9002);

    websocketpp::lib::error_code EC;
    Server_.start_accept(EC);

    Thread_.reset(new websocketpp::lib::thread(&ServerType::run, &Server_));
}

void NetworkManager::onMessage(websocketpp::connection_hdl, ServerType::message_ptr _Msg)
{
    json j = json::parse(_Msg->get_payload());

    std::cout << _Msg->get_payload() << std::endl;
    std::cout << "Extracted message: " << j["Message"] << std::endl;
}
