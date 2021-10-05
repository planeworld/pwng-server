#ifndef NETWORK_MESSAGE_BROKER_HPP
#define NETWORK_MESSAGE_BROKER_HPP

#include <functional>
#include <string>
#include <unordered_map>
// #include <unordered_set>

#include <concurrentqueue/concurrentqueue.h>
#include <entt/entity/registry.hpp>
#include "json_manager.hpp"
#include "network_message.hpp"

class NetworkMessageBroker
{

    public:

        explicit NetworkMessageBroker(entt::registry& _Reg,
                                      moodycamel::ConcurrentQueue<NetworkMessageClassified>* _QueueToSim,
                                      moodycamel::ConcurrentQueue<NetworkMessageParsed>* _QueueToNet,
                                      moodycamel::ConcurrentQueue<NetworkMessage>* _QueueOut);

        void process(const NetworkMessage& _m);
        void executeNet(const NetworkMessageParsed& _d);
        void executeSim(const NetworkMessageClassified& _d);

    private:

        void distribute(const NetworkMessageParsed& _d);
        NetworkMessageParsed parse(const NetworkMessage& _m) const;
        void sendError(JsonManager::ErrorType _e, JsonManager::ClientIDType _ClientID,
                       JsonManager::RequestIDType _MessageID, const char* _Data = "") const;
        void sendError(JsonManager::ClientIDType _ClientID, JsonManager::ParamCheckResult _r) const;
        void sendSuccess(JsonManager::ClientIDType _ClientID, JsonManager::RequestIDType _MessageID) const;

        void distributeCommand(const NetworkMessageParsed _m, NetworkMessageClassificationType _c, const std::string& _s);
        void sub(const NetworkMessageParsed _m, NetworkMessageClassificationType _c, const std::string& _s);
        void unsub(const NetworkMessageParsed _m, NetworkMessageClassificationType _c, const std::string& _s);

        entt::registry&  Reg_;

        std::unordered_map<std::string, std::function<void(const NetworkMessageParsed&, NetworkMessageClassificationType)>> Domains_;
        std::unordered_map<std::string, std::function<void(const NetworkMessageParsed&)>> ActionsMain_;
        std::unordered_map<std::string, std::function<void(const NetworkMessageParsed&)>> ActionsNet_;
        std::unordered_map<std::string, std::function<void(const NetworkMessageClassified&)>> ActionsSim_;

        moodycamel::ConcurrentQueue<NetworkMessageClassified>* QueueToSim_{nullptr};
        moodycamel::ConcurrentQueue<NetworkMessageParsed>* QueueToNet_{nullptr};
        moodycamel::ConcurrentQueue<NetworkMessage>* QueueOut_{nullptr};

        // Helpers, to handle string to enum conversion
        NetworkMessageClassificationType to_enum(const std::string& _s);

        const std::unordered_map<std::string, NetworkMessageClassificationType> SubscriptionTypeMap
        {
            {"evt", NetworkMessageClassificationType::EVT},
            {"s01", NetworkMessageClassificationType::S01},
            {"s05", NetworkMessageClassificationType::S05},
            {"s1", NetworkMessageClassificationType::S1},
            {"s5", NetworkMessageClassificationType::S5},
            {"s10", NetworkMessageClassificationType::S10}
        };

};

inline void NetworkMessageBroker::distributeCommand(const NetworkMessageParsed _m, NetworkMessageClassificationType _c, const std::string& _s)
{
    auto& Messages = Reg_.ctx<MessageHandler>();
    Messages.report("brk", _s+" requested", MessageHandler::INFO);
    DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
    QueueToSim_->enqueue({_m.ClientID, _c, _m.Payload});
}

inline void NetworkMessageBroker::sub(const NetworkMessageParsed _m, NetworkMessageClassificationType _c,
                                      const std::string& _s)
{
    auto& Messages = Reg_.ctx<MessageHandler>();
    Messages.report("brk", "Subscribe on " + _s + " requested", MessageHandler::INFO);
    DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
    QueueToSim_->enqueue({_m.ClientID, _c, _m.Payload});
}

inline void NetworkMessageBroker::unsub(const NetworkMessageParsed _m, NetworkMessageClassificationType _c,
                                        const std::string& _s)
{
    auto& Messages = Reg_.ctx<MessageHandler>();
    Messages.report("brk", "Unsubscribe from " + _s + " requested", MessageHandler::INFO);
    DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
    QueueToSim_->enqueue({_m.ClientID, _c, _m.Payload});
}

#endif // NETWORK_MESSAGE_BROKER_HPP
