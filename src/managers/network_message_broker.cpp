#include "network_message_broker.hpp"

#include "message_handler.hpp"
#include "network_manager.hpp"
#include "simulation_manager.hpp"
#include "subscription_components.hpp"

NetworkMessageBroker::NetworkMessageBroker(entt::registry& _Reg,
                    moodycamel::ConcurrentQueue<NetworkMessageClassified>* _QueueToSim,
                    moodycamel::ConcurrentQueue<NetworkMessageParsed>* _QueueToNet,
                    moodycamel::ConcurrentQueue<NetworkMessage>* _QueueOut) :
                    Reg_(_Reg),
                    QueueToSim_(_QueueToSim),
                    QueueToNet_(_QueueToNet),
                    QueueOut_(_QueueOut)
{
    //--- Distribution of messagas ---//

    auto& Messages = Reg_.ctx<MessageHandler>();
    Domains_.insert({"cmd_accelerate_simulation", [&](const NetworkMessageParsed& _m, NetworkMessageClassificationType _c)
    {
        Messages.report("brk", "Simulation acceleration requested", MessageHandler::INFO);
        DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
        QueueToSim_->enqueue({_m.ClientID, _c, _m.Payload});
    }});
    Domains_.insert({"cmd_start_simulation", [&](const NetworkMessageParsed& _m, NetworkMessageClassificationType _c)
    {
        Messages.report("brk", "Simulation start requested", MessageHandler::INFO);
        DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
        QueueToSim_->enqueue({_m.ClientID, _c, _m.Payload});
    }});
    Domains_.insert({"cmd_stop_simulation", [&](const NetworkMessageParsed& _m, NetworkMessageClassificationType _c)
    {
        Messages.report("brk", "Simulation stop requested", MessageHandler::INFO);
        DBLK(Messages.report("brk", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
        QueueToSim_->enqueue({_m.ClientID, _c, _m.Payload});
    }});
    Domains_.insert({"sub_dynamic_data", [&](const NetworkMessageParsed& _m, NetworkMessageClassificationType _c)
    {
        this->sub(_m, _c, "dynamic data");
    }});
    Domains_.insert({"uns_dynamic_data", [&](const NetworkMessageParsed& _m, NetworkMessageClassificationType _c)
    {
        this->unsub(_m, _c, "dynamic data");
    }});
    Domains_.insert({"sub_galaxy_data", [&](const NetworkMessageParsed& _m, NetworkMessageClassificationType _c)
    {
        this->sub(_m, _c, "galaxy data");
    }});
    Domains_.insert({"uns_galaxy_data", [&](const NetworkMessageParsed& _m, NetworkMessageClassificationType _c)
    {
        this->unsub(_m, _c, "galaxy data");
    }});
    Domains_.insert({"sub_perf_stats", [&](const NetworkMessageParsed& _m, NetworkMessageClassificationType _c)
    {
        this->sub(_m, _c, "performance stats");
    }});
    Domains_.insert({"uns_perf_stats", [&](const NetworkMessageParsed& _m, NetworkMessageClassificationType _c)
    {
        this->unsub(_m, _c, "performance stats");
    }});
    Domains_.insert({"sub_sim_stats", [&](const NetworkMessageParsed& _m, NetworkMessageClassificationType _c)
    {
        this->sub(_m, _c, "sim stats");
    }});
    Domains_.insert({"uns_sim_stats", [&](const NetworkMessageParsed& _m, NetworkMessageClassificationType _c)
    {
        this->unsub(_m, _c, "sim stats");
    }});
    Domains_.insert({"sub_system", [&](const NetworkMessageParsed& _m, NetworkMessageClassificationType _c)
    {
        this->sub(_m, _c, "system");
    }});
    Domains_.insert({"uns_system", [&](const NetworkMessageParsed& _m, NetworkMessageClassificationType _c)
    {
        this->unsub(_m, _c, "system");
    }});

    //--- Processing of messagas ---//
    //
    ActionsMain_.insert({"cmd_shutdown", [&](const NetworkMessageParsed& _d)
    {
        Messages.report("brk", "Server shutdown requested", MessageHandler::INFO);
        auto& Json = Reg_.ctx<JsonManager>();
        auto r = Json.checkParams(_d.Payload, {});
        if (r.Success)
        {
            DBLK(Messages.report("brk", "Shutting down simulation...", MessageHandler::DEBUG_L1);)
            Reg_.ctx<SimulationManager>().shutdown();
            std::this_thread::sleep_for(std::chrono::seconds(2));
            DBLK(Messages.report("brk", "Shutting down network...", MessageHandler::DEBUG_L1);)
            Reg_.ctx<NetworkManager>().stop();
            DBLK(Messages.report("brk", "...done", MessageHandler::DEBUG_L1);)
            this->sendSuccess(_d.ClientID, JsonManager::getID(_d.Payload));
        }
        else
        {
            this->sendError(_d.ClientID, r);
        }
    }});

    ActionsSim_.insert({"cmd_accelerate_simulation", [&](const NetworkMessageClassified& _d)
    {
        DBLK(Messages.report("brk", "Accelerating simulation", MessageHandler::DEBUG_L1);)
        auto& Json = Reg_.ctx<JsonManager>();
        auto r = Json.checkParams(_d.Payload, {JsonManager::ParamsType::NUMBER});
        if (r.Success)
        {
            auto Accel = JsonManager::getParams(_d.Payload)[0].GetDouble();
            if (Accel > 1.0e6)
            {
                Accel = 1.0e6;
                Json.createResult()
                    .beginObject()
                    .addNamedValue("success", true)
                    .addNamedValue("notification", "Out of bounds, valid interval is [0.1, 1.0e6]. Clamping value.")
                    .endObject()
                    .finalise(JsonManager::getID(_d.Payload));
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
                    .finalise(JsonManager::getID(_d.Payload));
            }
            else
            {
                this->sendSuccess(_d.ClientID, JsonManager::getID(_d.Payload));
            }
            Reg_.ctx<SimulationManager>().setAccel(Accel);
        }
        else
        {
            this->sendError(_d.ClientID, r);
        }
    }});
    ActionsSim_.insert({"cmd_start_simulation", [&](const NetworkMessageClassified& _d)
    {
        DBLK(Messages.report("brk", "Starting simulation", MessageHandler::DEBUG_L1);)
        auto& Json = Reg_.ctx<JsonManager>();
        auto r = Json.checkParams(_d.Payload, {});
        if (r.Success)
        {
            Reg_.ctx<SimulationManager>().start();
            this->sendSuccess(_d.ClientID, JsonManager::getID(_d.Payload));
        }
        else
        {
            this->sendError(_d.ClientID, r);
        }
    }});
    ActionsSim_.insert({"cmd_stop_simulation", [&](const NetworkMessageClassified& _d)
    {
        DBLK(Messages.report("brk", "Stopping simulation", MessageHandler::DEBUG_L1);)
        auto& Json = Reg_.ctx<JsonManager>();
        auto r = Json.checkParams(_d.Payload, {});
        if (r.Success)
        {
            Reg_.ctx<SimulationManager>().stop();
            this->sendSuccess(_d.ClientID, JsonManager::getID(_d.Payload));
        }
        else
        {
            this->sendError(_d.ClientID, r);
        }
    }});
    ActionsSim_.insert({"sub_dynamic_data", [&](const NetworkMessageClassified& _d)
    {
        DBLK(Messages.report("brk", "Subscribing on dynamic data", MessageHandler::DEBUG_L1);)
        Reg_.emplace_or_replace<DynamicDataSubscriptionComponent>(_d.ClientID);
    }});
    ActionsSim_.insert({"uns_dynamic_data", [&](const NetworkMessageClassified& _d)
    {
        DBLK(Messages.report("brk", "Unsubscribing from dynamic data", MessageHandler::DEBUG_L1);)
        Reg_.remove<DynamicDataSubscriptionComponent>(_d.ClientID);
    }});
    ActionsSim_.insert({"sub_galaxy_data", [&](const NetworkMessageClassified& _d)
    {
        DBLK(Messages.report("brk", "Subscribing on galaxy data", MessageHandler::DEBUG_L1);)
        if (_d.Class == NetworkMessageClassificationType::EVT)
        {
            Reg_.emplace_or_replace<GalaxyDataSubscriptionComponent>(_d.ClientID);
        }
        else
        {
            Messages.report("brk", "Invalid subscription type", MessageHandler::WARNING);
            this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d.Payload), "Allowed subscription types: [evt]");
        }

    }});
    ActionsSim_.insert({"uns_galaxy_data", [&](const NetworkMessageClassified& _d)
    {
        DBLK(Messages.report("brk", "Unsubscribing from galaxy data", MessageHandler::DEBUG_L1);)
        if (_d.Class == NetworkMessageClassificationType::EVT)
        {
            Reg_.remove<GalaxyDataSubscriptionComponent>(_d.ClientID);
        }
        else
        {
            Messages.report("brk", "Invalid subscription type", MessageHandler::WARNING);
            this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d.Payload), "Allowed subscription types: [evt]");
        }

    }});
    ActionsSim_.insert({"sub_perf_stats", [&](const NetworkMessageClassified& _d)
    {
        DBLK(Messages.report("brk", "Subscribing on performance stats", MessageHandler::DEBUG_L1);)
        switch (_d.Class)
        {
            case NetworkMessageClassificationType::S01:
                Reg_.emplace_or_replace<PerformanceStatsSubscriptionTag01>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S05:
                Reg_.emplace_or_replace<PerformanceStatsSubscriptionTag05>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S1:
                Reg_.emplace_or_replace<PerformanceStatsSubscriptionTag1>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S5:
                Reg_.emplace_or_replace<PerformanceStatsSubscriptionTag5>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S10:
                Reg_.emplace_or_replace<PerformanceStatsSubscriptionTag10>(_d.ClientID);
                break;
            default:
                Messages.report("brk", "Invalid subscription type", MessageHandler::WARNING);
                this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d.Payload), "Allowed subscription types: [s01, s05, s1, s5, s10]");
                break;
        }
        Reg_.emplace_or_replace<PerformanceStatsSubscriptionTag01>(_d.ClientID);
    }});
    ActionsSim_.insert({"uns_perf_stats", [&](const NetworkMessageClassified& _d)
    {
        DBLK(Messages.report("brk", "Unsubscribing from performance stats", MessageHandler::DEBUG_L1);)
        switch (_d.Class)
        {
            case NetworkMessageClassificationType::S01:
                Reg_.remove<PerformanceStatsSubscriptionTag01>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S05:
                Reg_.remove<PerformanceStatsSubscriptionTag05>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S1:
                Reg_.remove<PerformanceStatsSubscriptionTag1>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S5:
                Reg_.remove<PerformanceStatsSubscriptionTag5>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S10:
                Reg_.remove<PerformanceStatsSubscriptionTag10>(_d.ClientID);
                break;
            default:
                Messages.report("brk", "Invalid subscription type", MessageHandler::WARNING);
                this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d.Payload), "Allowed subscription types: [s01, s05, s1, s5, s10]");
                break;
        }
        Reg_.emplace_or_replace<PerformanceStatsSubscriptionTag01>(_d.ClientID);
    }});
    ActionsSim_.insert({"sub_sim_stats", [&](const NetworkMessageClassified& _d)
    {
        DBLK(Messages.report("brk", "Subscribing on simulation stats", MessageHandler::DEBUG_L1);)
        switch (_d.Class)
        {
            case NetworkMessageClassificationType::S01:
                Reg_.emplace_or_replace<SimStatsSubscriptionTag01>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S05:
                Reg_.emplace_or_replace<SimStatsSubscriptionTag05>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S1:
                Reg_.emplace_or_replace<SimStatsSubscriptionTag1>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S5:
                Reg_.emplace_or_replace<SimStatsSubscriptionTag5>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S10:
                Reg_.emplace_or_replace<SimStatsSubscriptionTag10>(_d.ClientID);
                break;
            default:
                Messages.report("brk", "Invalid subscription type", MessageHandler::WARNING);
                this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d.Payload), "Allowed subscription types: [s01, s05, s1, s5, s10]");
                break;
        }
    }});
    ActionsSim_.insert({"uns_sim_stats", [&](const NetworkMessageClassified& _d)
    {
        DBLK(Messages.report("brk", "Unsubscribing from simulation stats", MessageHandler::DEBUG_L1);)
        switch (_d.Class)
        {
            case NetworkMessageClassificationType::S01:
                Reg_.remove<SimStatsSubscriptionTag01>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S05:
                Reg_.remove<SimStatsSubscriptionTag05>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S1:
                Reg_.remove<SimStatsSubscriptionTag1>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S5:
                Reg_.remove<SimStatsSubscriptionTag5>(_d.ClientID);
                break;
            case NetworkMessageClassificationType::S10:
                Reg_.remove<SimStatsSubscriptionTag10>(_d.ClientID);
                break;
            default:
                Messages.report("brk", "Invalid subscription type", MessageHandler::WARNING);
                this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d.Payload), "Allowed subscription types: [s01, s05, s1, s5, s10]");
                break;
        }
    }});
}

void NetworkMessageBroker::process(const NetworkMessage& _m)
{
    auto _d = this->parse(_m);

    auto& Messages = Reg_.ctx<MessageHandler>();

    if (JsonManager::checkRequest(_d.Payload) == true)
    {
        const auto c = JsonManager::getMethod(_d.Payload);
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
        if (JsonManager::checkID(_d.Payload) == true)
        {
            this->sendError(JsonManager::ErrorType::REQUEST, _d.ClientID, JsonManager::getID(_d.Payload), "Missing field <method>");
        }
        else
        {
            if (JsonManager::checkMethod(_d.Payload))
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

void NetworkMessageBroker::executeNet(const NetworkMessageParsed& _d)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    const auto c = JsonManager::getMethod(_d.Payload);
    if (ActionsNet_.find(c) != ActionsNet_.end())
    {
        ActionsNet_[c](_d);
        this->sendSuccess(_d.ClientID, (*_d.Payload)["id"].GetUint());
    }
    else
    {
        Messages.report("brk", "Unknown method "+std::string(c), MessageHandler::WARNING);
        this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d.Payload));
    }
}

void NetworkMessageBroker::executeSim(const NetworkMessageClassified& _d)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    const auto c = JsonManager::getMethod(_d.Payload);
    if (ActionsSim_.find(c) != ActionsSim_.end())
    {
        ActionsSim_[c](_d);
    }
    else
    {
        Messages.report("brk", "Unknown method "+std::string(c), MessageHandler::WARNING);
        this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d.Payload));
    }
}

void NetworkMessageBroker::distribute(const NetworkMessageParsed& _d)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    const auto c = std::string(JsonManager::getMethod(_d.Payload));
    const auto Prefix = c.substr(0,3);

    NetworkMessageClassificationType ClassificationType{NetworkMessageClassificationType::CMD};
    bool IsValidPrefixSuffix{false};

    if (Prefix == "cmd")
    {
        IsValidPrefixSuffix = true;
    }
    else if (Prefix == "sub" || Prefix == "uns")
    {
        auto p = c.find_last_of("_");
        auto f = c.substr(p+1);
        JsonManager::replaceMethod(_d.Payload, c.substr(0,p).c_str());
        ClassificationType = to_enum(f);
        if (ClassificationType != NetworkMessageClassificationType::INVALID)
        {
            IsValidPrefixSuffix = true;
            DBLK(
            {
                Messages.report("brk", "Requested subscription frequency: "+f, MessageHandler::DEBUG_L1);
            })
        }
        else
        {
            Messages.report("brk", "Unknown suffix for subscription frequency", MessageHandler::WARNING);
            this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d.Payload));
        }
    }
    else
    {
        Messages.report("brk", "Unknown prefix or missing, should be <cmd_>/<sub_>/<uns_>", MessageHandler::WARNING);
        this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d.Payload));
    }
    if (IsValidPrefixSuffix)
    {
        auto Method = JsonManager::getMethod(_d.Payload);
        if (Domains_.find(Method) != Domains_.end())
        {
            Domains_[Method](_d, ClassificationType);
        }
        else
        {
            Messages.report("brk", "Unknown method "+std::string(Method), MessageHandler::WARNING);
            this->sendError(JsonManager::ErrorType::METHOD, _d.ClientID, JsonManager::getID(_d.Payload));
        }
    }
}

NetworkMessageParsed NetworkMessageBroker::parse(const NetworkMessage& _m) const
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    DBLK(Messages.report("brk", "Parsing message", MessageHandler::DEBUG_L1);)

    const auto d = std::make_shared<Document>();

    ParseResult r = d->Parse(_m.Payload.c_str());
    if (!r)
    {
        this->sendError(JsonManager::ErrorType::PARSE, _m.ClientID, JsonManager::getID(d));
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return {_m.ClientID, d};
}

void NetworkMessageBroker::sendError(JsonManager::ClientIDType _ClientID, JsonManager::ParamCheckResult _r) const
{
    auto& Json = Reg_.ctx<JsonManager>();
    Json.createError(_r.Error, _r.Explanation.c_str())
        .finalise(_r.RequestID);
    QueueOut_->enqueue({_ClientID, Json.getString()});
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

NetworkMessageClassificationType NetworkMessageBroker::to_enum(const std::string& _s)
{
    auto it = SubscriptionTypeMap.find(_s);
    if (it != SubscriptionTypeMap.end()) return it->second;
    else return NetworkMessageClassificationType::INVALID;
}
