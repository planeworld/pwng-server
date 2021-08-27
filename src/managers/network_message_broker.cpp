#include "network_message_broker.hpp"

#include "message_handler.hpp"
#include "network_manager.hpp"
#include "simulation_manager.hpp"
#include "subscription_components.hpp"

NetworkMessageBroker::NetworkMessageBroker(entt::registry& _Reg,
                    moodycamel::ConcurrentQueue<NetworkDocument>* _QueueToSim,
                    moodycamel::ConcurrentQueue<NetworkDocument>* _QueueToNet,
                    moodycamel::ConcurrentQueue<NetworkMessage>* _QueueOut) :
                    Reg_(_Reg),
                    QueueToSim_(_QueueToSim),
                    QueueToNet_(_QueueToNet),
                    QueueOut_(_QueueOut)
{
    auto& Messages = Reg_.ctx<MessageHandler>();
    Domains_.insert({"start_simulation", [&](const NetworkDocument& _m)
    {
        Messages.report("brk", "Simulation start requested", MessageHandler::INFO);
        DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
        QueueToSim_->enqueue(_m);
    }});
    Domains_.insert({"stop_simulation", [&](const NetworkDocument& _m)
    {
        Messages.report("brk", "Simulation stop requested", MessageHandler::INFO);
        DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
        QueueToSim_->enqueue(_m);
    }});
    Domains_.insert({"sub_dynamic_data", [&](const NetworkDocument& _m)
    {
        Messages.report("brk", "Subscribe on dynamic data requested", MessageHandler::INFO);
        DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
        QueueToSim_->enqueue(_m);
    }});
    Domains_.insert({"sub_galaxy_data", [&](const NetworkDocument& _m)
    {
        Messages.report("brk", "Subscribe on galaxy data requested", MessageHandler::INFO);
        DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
        QueueToSim_->enqueue(_m);
    }});
    Domains_.insert({"sub_perf_stats", [&](const NetworkDocument& _m)
    {
        Messages.report("brk", "Subscribe on performance stats requested", MessageHandler::INFO);
        DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
        QueueToSim_->enqueue(_m);
    }});
    Domains_.insert({"sub_sim_stats", [&](const NetworkDocument& _m)
    {
        Messages.report("brk", "Subscribe on sim stats requested", MessageHandler::INFO);
        DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
        QueueToSim_->enqueue(_m);
    }});
    Domains_.insert({"sub_system", [&](const NetworkDocument& _m)
    {
        Messages.report("brk", "Subscribe on star system requested", MessageHandler::INFO);
        DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
        QueueToSim_->enqueue(_m);
    }});

    ActionsMain_.insert({"shutdown", [&](const entt::entity _e)
    {
        Messages.report("brk", "Server shutdown requested", MessageHandler::INFO);
        DBLK(Messages.report("brk", "Shutting down simulation...", MessageHandler::DEBUG_L1);)
        Reg_.ctx<SimulationManager>().shutdown();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        DBLK(Messages.report("brk", "Shutting down network...", MessageHandler::DEBUG_L1);)
        Reg_.ctx<NetworkManager>().stop();
        DBLK(Messages.report("brk", "...done", MessageHandler::DEBUG_L1);)
    }});

    ActionsSim_.insert({"sub_dynamic_data", [&](const entt::entity _e)
    {
        DBLK(Messages.report("brk", "Subscribing on dynamic data", MessageHandler::DEBUG_L1);)
        Reg_.emplace_or_replace<DynamicDataSubscriptionComponent>(_e);
    }});
    ActionsSim_.insert({"sub_galaxy_data", [&](const entt::entity _e)
    {
        DBLK(Messages.report("brk", "Subscribing on galaxy data", MessageHandler::DEBUG_L1);)
        Reg_.emplace_or_replace<GalaxyDataSubscriptionTag>(_e);
    }});
    ActionsSim_.insert({"sub_perf_stats", [&](const entt::entity _e)
    {
        DBLK(Messages.report("brk", "Subscribing on performance stats", MessageHandler::DEBUG_L1);)
        Reg_.emplace_or_replace<PerformanceStatsSubscriptionTag01>(_e);
    }});
    ActionsSim_.insert({"sub_sim_stats", [&](const entt::entity _e)
    {
        DBLK(Messages.report("brk", "Subscribing on simulation stats", MessageHandler::DEBUG_L1);)
        Reg_.emplace_or_replace<SimStatsSubscriptionTag01>(_e);
    }});
    ActionsSim_.insert({"start_simulation", [&](const entt::entity _e)
    {
        DBLK(Messages.report("brk", "Starting simulation", MessageHandler::DEBUG_L1);)
        Reg_.ctx<SimulationManager>().start();
    }});
    ActionsSim_.insert({"stop_simulation", [&](const entt::entity _e)
    {
        DBLK(Messages.report("brk", "Stopping simulation", MessageHandler::DEBUG_L1);)
        Reg_.ctx<SimulationManager>().stop();
    }});
}

void NetworkMessageBroker::process(const NetworkDocument& _d)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    const auto c = (*_d.Payload)["method"].GetString();
    if (ActionsMain_.find(c) != ActionsMain_.end())
    {
        DBLK(Messages.report("brk", "Processing message", MessageHandler::DEBUG_L3);)
        ActionsMain_[c](_d.ClientID);
    }
    else
    {
        DBLK(Messages.report("brk", "Distributing message", MessageHandler::DEBUG_L1);)
        this->distribute(_d);
    }
}

void NetworkMessageBroker::executeNet(const NetworkDocument& _d)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    const auto c = (*_d.Payload)["method"].GetString();
    if (ActionsNet_.find(c) != ActionsNet_.end())
    {
        ActionsNet_[c](_d.ClientID);
        this->sendSuccess(_d.ClientID, (*_d.Payload)["id"].GetUint());
    }
    else
    {
        Messages.report("brk", "Unknown message"+std::string(c), MessageHandler::WARNING);
        this->sendError(_d.ClientID, (*_d.Payload)["id"].GetUint());
    }
}

void NetworkMessageBroker::executeSim(const NetworkDocument& _d)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    const auto c = (*_d.Payload)["method"].GetString();
    if (ActionsSim_.find(c) != ActionsSim_.end())
    {
        ActionsSim_[c](_d.ClientID);
        this->sendSuccess(_d.ClientID, (*_d.Payload)["id"].GetUint());
    }
    else
    {
        Messages.report("brk", "Unknown message"+std::string(c), MessageHandler::WARNING);
        this->sendError(_d.ClientID, (*_d.Payload)["id"].GetUint());
    }
}

NetworkDocument NetworkMessageBroker::parse(const NetworkMessage& _m) const
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    DBLK(Messages.report("brk", "Parsing message", MessageHandler::DEBUG_L1);)

    const auto d = std::make_shared<Document>();
    d->Parse(_m.Payload.c_str());

    return {_m.ClientID, d};
}

void NetworkMessageBroker::distribute(const NetworkDocument& _d)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    const auto c = (*_d.Payload)["method"].GetString();
    if (Domains_.find(c) != Domains_.end())
    {
        Domains_[c](_d);
    }
    else
    {
        Messages.report("brk", "Unknown message "+std::string(c), MessageHandler::WARNING);
        this->sendError(_d.ClientID, (*_d.Payload)["id"].GetUint());
    }
}

void NetworkMessageBroker::sendError(JsonManager::ClientIDType _ClientID, JsonManager::RequestIDType _MessageID) const
{
    auto& Json = Reg_.ctx<JsonManager>();
    Json.createError()
        .finalise(_MessageID);
    QueueOut_->enqueue({_ClientID, Json.getString()});
}


void NetworkMessageBroker::sendSuccess(JsonManager::ClientIDType _ClientID, JsonManager::RequestIDType _MessageID) const
{
    auto& Json = Reg_.ctx<JsonManager>();
    Json.createResult(true)
        .finalise(_MessageID);
    QueueOut_->enqueue({_ClientID, Json.getString()});
}
