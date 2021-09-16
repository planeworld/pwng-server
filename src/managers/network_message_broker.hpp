#ifndef NETWORK_MESSAGE_BROKER_HPP
#define NETWORK_MESSAGE_BROKER_HPP

#include <functional>
#include <string>
#include <unordered_map>

#include <concurrentqueue/concurrentqueue.h>
#include <entt/entity/registry.hpp>
#include "json_manager.hpp"
#include "network_message.hpp"

class NetworkMessageBroker
{

    public:

        explicit NetworkMessageBroker(entt::registry& _Reg,
                                      moodycamel::ConcurrentQueue<NetworkDocument>* _QueueToSim,
                                      moodycamel::ConcurrentQueue<NetworkDocument>* _QueueToNet,
                                      moodycamel::ConcurrentQueue<NetworkMessage>* _QueueOut);

        void process(const NetworkDocument& _d);
        void executeNet(const NetworkDocument& _d);
        void executeSim(const NetworkDocument& _d);
        NetworkDocument parse(const NetworkMessage& _m) const;


    private:

        void distribute(const NetworkDocument& _d);
        void sendError(JsonManager::ErrorType _e, JsonManager::ClientIDType _ClientID, JsonManager::RequestIDType _MessageID) const;
        void sendError(JsonManager::ParamCheckResult _r) const;
        void sendSuccess(JsonManager::ClientIDType _ClientID, JsonManager::RequestIDType _MessageID) const;

        entt::registry&  Reg_;

        std::unordered_map<std::string, std::function<void(const NetworkDocument&)>> Domains_;
        std::unordered_map<std::string, std::function<void(const NetworkDocument&)>> ActionsMain_;
        std::unordered_map<std::string, std::function<void(const NetworkDocument&)>> ActionsNet_;
        std::unordered_map<std::string, std::function<void(const NetworkDocument&)>> ActionsSim_;

        moodycamel::ConcurrentQueue<NetworkDocument>* QueueToSim_{nullptr};
        moodycamel::ConcurrentQueue<NetworkDocument>* QueueToNet_{nullptr};
        moodycamel::ConcurrentQueue<NetworkMessage>* QueueOut_{nullptr};

};

#endif // NETWORK_MESSAGE_BROKER_HPP
