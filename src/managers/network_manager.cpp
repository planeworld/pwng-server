#include "network_manager.hpp"

#include <nlohmann/json.hpp>

void NetworkManager::init(moodycamel::ConcurrentQueue<std::string>* const _InputQueue,
                          moodycamel::ConcurrentQueue<std::string>* const _OutputQueue)
{
    InputQueue_ = _InputQueue;
    OutputQueue_ = _OutputQueue;

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
    InputQueue_->enqueue(_Msg->get_payload());
}
