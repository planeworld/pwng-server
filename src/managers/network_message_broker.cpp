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

    //--- Distribution of messagas ---//

    auto& Messages = Reg_.ctx<MessageHandler>();
    Domains_.insert({"cmd_accelerate_simulation", [&](const NetworkDocument& _m)
    {
        Messages.report("brk", "Simulation acceleration requested", MessageHandler::INFO);
        DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
        QueueToSim_->enqueue(_m);
    }});
    Domains_.insert({"cmd_start_simulation", [&](const NetworkDocument& _m)
    {
        Messages.report("brk", "Simulation start requested", MessageHandler::INFO);
        DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
        QueueToSim_->enqueue(_m);
    }});
    Domains_.insert({"cmd_stop_simulation", [&](const NetworkDocument& _m)
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

    //--- Processing of messagas ---//
    //
    ActionsMain_.insert({"cmd_shutdown", [&](const NetworkDocument& _d)
    {
        Messages.report("brk", "Server shutdown requested", MessageHandler::INFO);
        auto& Json = Reg_.ctx<JsonManager>();
        auto r = Json.checkParams(_d, {});
        if (r.Success)
        {
            DBLK(Messages.report("brk", "Shutting down simulation...", MessageHandler::DEBUG_L1);)
            Reg_.ctx<SimulationManager>().shutdown();
            std::this_thread::sleep_for(std::chrono::seconds(2));
            DBLK(Messages.report("brk", "Shutting down network...", MessageHandler::DEBUG_L1);)
            Reg_.ctx<NetworkManager>().stop();
            DBLK(Messages.report("brk", "...done", MessageHandler::DEBUG_L1);)
            this->sendSuccess(_d.ClientID, JsonManager::getID(_d));
        }
        else
        {
            this->sendError(r);
        }
    }});

    ActionsSim_.insert({"cmd_accelerate_simulation", [&](const NetworkDocument& _d)
    {
        DBLK(Messages.report("brk", "Accelerating simulation", MessageHandler::DEBUG_L1);)
        auto& Json = Reg_.ctx<JsonManager>();
        auto r = Json.checkParams(_d, {JsonManager::ParamsType::NUMBER});
        if (r.Success)
        {
            auto Accel = JsonManager::getParams(_d)[0].GetDouble();
            if (Accel > 1.0e6)
            {
                Accel = 1.0e6;
                Json.createResult()
                    .beginObject()
                    .addNamedValue("success", true)
                    .addNamedValue("notification", "Out of bounds, valid interval is [0.1, 1.0e6]. Clamping value.")
                    .endObject()
                    .finalise(JsonManager::getID(_d));
                QueueOut_->enqueue({_d.ClientID, Json.getString()});
            }
            else if (Accel < 0.1)
            {
                Accel = 0.1;
                Json.createResult()
                    .beginObject()
                    .addNamedValue("success", true)
                    .addNamedValue("notification", "Out of bounds, valid interval is [0.1, 1.0e6]. Clamping value.")
                    .endObject()
                    .finalise(JsonManager::getID(_d));
            }
            else
            {
                this->sendSuccess(_d.ClientID, JsonManager::getID(_d));
            }
            Reg_.ctx<SimulationManager>().setAccel(Accel);
        }
        else
        {
            this->sendError(r);
        }
    }});
    ActionsSim_.insert({"cmd_start_simulation", [&](const NetworkDocument& _d)
    {
        DBLK(Messages.report("brk", "Starting simulation", MessageHandler::DEBUG_L1);)
        auto& Json = Reg_.ctx<JsonManager>();
        auto r = Json.checkParams(_d, {});
        if (r.Success)
        {
            Reg_.ctx<SimulationManager>().start();
            this->sendSuccess(_d.ClientID, JsonManager::getID(_d));
        }
        else
        {
            this->sendError(r);
        }
    }});
    ActionsSim_.insert({"cmd_stop_simulation", [&](const NetworkDocument& _d)
    {
        DBLK(Messages.report("brk", "Stopping simulation", MessageHandler::DEBUG_L1);)
        auto& Json = Reg_.ctx<JsonManager>();
        auto r = Json.checkParams(_d, {});
        if (r.Success)
        {
            Reg_.ctx<SimulationManager>().stop();
            this->sendSuccess(_d.ClientID, JsonManager::getID(_d));
        }
        else
        {
            this->sendError(r);
        }
    }});
    ActionsSim_.insert({"sub_dynamic_data", [&](const NetworkDocument& _d)
    {
        DBLK(Messages.report("brk", "Subscribing on dynamic data", MessageHandler::DEBUG_L1);)
        Reg_.emplace_or_replace<DynamicDataSubscriptionComponent>(_d.ClientID);
    }});
    ActionsSim_.insert({"sub_galaxy_data", [&](const NetworkDocument& _d)
    {
        DBLK(Messages.report("brk", "Subscribing on galaxy data", MessageHandler::DEBUG_L1);)
        Reg_.emplace_or_replace<GalaxyDataSubscriptionComponent>(_d.ClientID);
    }});
    ActionsSim_.insert({"sub_perf_stats", [&](const NetworkDocument& _d)
    {
        DBLK(Messages.report("brk", "Subscribing on performance stats", MessageHandler::DEBUG_L1);)
        Reg_.emplace_or_replace<PerformanceStatsSubscriptionTag01>(_d.ClientID);
    }});
    ActionsSim_.insert({"sub_sim_stats", [&](const NetworkDocument& _d)
    {
        DBLK(Messages.report("brk", "Subscribing on simulation stats", MessageHandler::DEBUG_L1);)
        Reg_.emplace_or_replace<SimStatsSubscriptionTag01>(_d.ClientID);
    }});
}

void NetworkMessageBroker::process(const NetworkDocument& _d)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    if (JsonManager::checkRequest(_d) == true)
    {
        const auto c = JsonManager::getMethod(_d);
        if (ActionsMain_.find(c) != ActionsMain_.end())
        {
            DBLK(Messages.report("brk", "Processing message", MessageHandler::DEBUG_L3);)
            ActionsMain_[c](_d);
        }
        else
        {
            DBLK(Messages.report("brk", "Distributing message", MessageHandler::DEBUG_L1);)
            this->distribute(_d);
        }
    }
    else
    {
        Messages.report("brk", "Invalid jsonrpc request from client "+std::to_string(entt::to_integral(_d.ClientID)), MessageHandler::WARNING);
        if (JsonManager::checkID(_d) == true)
        {
            this->sendError(JsonManager::ErrorType::REQUEST, _d.ClientID, JsonManager::getID(_d), "Missing field <method>");
        }
        else
        {
            if (JsonManager::checkMethod(_d))
            {
                this->sendError(JsonManager::ErrorType::REQUEST, _d.ClientID, 0, "Missing fields <id>");
            }
            else
            {
                this->sendError(JsonManager::ErrorType::REQUEST, _d.ClientID, 0, "Missing field <method>/<id>");
            }
        }
    }
}

void NetworkMessageBroker::executeNet(const NetworkDocument& _d)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    const auto c = JsonManager::getMethod(_d);
    if (ActionsNet_.find(c) != ActionsNet_.end())
    {
        ActionsNet_[c](_d);
        this->sendSuccess(_d.ClientID, (*_d.Payload)["id"].GetUint());
    }
    else
    {
        Messages.report("brk", "Unknown method "+std::string(c), MessageHandler::WARNING);
        this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d));
    }
}

void NetworkMessageBroker::executeSim(const NetworkDocument& _d)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    const auto c = JsonManager::getMethod(_d);
    if (ActionsSim_.find(c) != ActionsSim_.end())
    {
        ActionsSim_[c](_d);
    }
    else
    {
        Messages.report("brk", "Unknown method "+std::string(c), MessageHandler::WARNING);
        this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d));
    }
}

NetworkDocument NetworkMessageBroker::parse(const NetworkMessage& _m) const
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    DBLK(Messages.report("brk", "Parsing message", MessageHandler::DEBUG_L1);)

    const auto d = std::make_shared<Document>();

    ParseResult r = d->Parse(_m.Payload.c_str());
    if (!r)
    {
        this->sendError(JsonManager::ErrorType::PARSE, _m.ClientID, JsonManager::getID({_m.ClientID, d}));
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return {_m.ClientID, d};
}

void NetworkMessageBroker::distribute(const NetworkDocument& _d)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    const auto c = JsonManager::getMethod(_d);
    if (Domains_.find(c) != Domains_.end())
    {
        Domains_[c](_d);
    }
    else
    {
        Messages.report("brk", "Unknown method "+std::string(c), MessageHandler::WARNING);
        this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d));
    }
}

void NetworkMessageBroker::sendError(JsonManager::ParamCheckResult _r) const
{
    auto& Json = Reg_.ctx<JsonManager>();
    Json.createError(_r.Error, _r.Explanation.c_str())
        .finalise(_r.RequestID);
    QueueOut_->enqueue({_r.ClientID, Json.getString()});
}

void NetworkMessageBroker::sendError(JsonManager::ErrorType _e, JsonManager::ClientIDType _ClientID, JsonManager::RequestIDType _MessageID, const char* _Data) const
{
    auto& Json = Reg_.ctx<JsonManager>();
    Json.createError(_e, _Data)
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
