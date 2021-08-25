#ifndef NETWORK_MESSAGE_BROKER_HPP
#define NETWORK_MESSAGE_BROKER_HPP

#include <functional>
#include <string>
#include <unordered_map>

#include <concurrentqueue/concurrentqueue.h>
#include <entt/entity/registry.hpp>
#include "network_message.hpp"

class NetworkMessageBroker
{

    public:

        explicit NetworkMessageBroker(entt::registry& _Reg,
                                      moodycamel::ConcurrentQueue<NetworkDocument>* _QueueToSim,
                                      moodycamel::ConcurrentQueue<NetworkDocument>* _QueueToNet);

        void process(const NetworkDocument& _d);
        void executeNet(const NetworkDocument& _d);
        void executeSim(const NetworkDocument& _d);
        NetworkDocument parse(const NetworkMessage& _m) const;


    private:

        void distribute(const NetworkDocument& _d);

        entt::registry&  Reg_;

        std::unordered_map<std::string, std::function<void(const NetworkDocument&)>> Domains_;
        std::unordered_map<std::string, std::function<void(const entt::entity)>> ActionsMain_;
        std::unordered_map<std::string, std::function<void(const entt::entity)>> ActionsNet_;
        std::unordered_map<std::string, std::function<void(const entt::entity)>> ActionsSim_;

        moodycamel::ConcurrentQueue<NetworkDocument>* QueueToSim_{nullptr};
        moodycamel::ConcurrentQueue<NetworkDocument>* QueueToNet_{nullptr};

};

#endif // NETWORK_MESSAGE_BROKER_HPP
