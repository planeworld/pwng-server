#include <iostream>
#include <sstream>

#include <argagg/argagg.hpp>
#include <concurrentqueue/concurrentqueue.h>
#include <entt/entity/registry.hpp>
#include <nlohmann/json.hpp>

#include "message_handler.hpp"
#include "network_manager.hpp"
#include "position_component.hpp"
#include "simulation_manager.hpp"
#include "subscription_components.hpp"
#include "velocity_component.hpp"

using json = nlohmann::json;

constexpr int PWNG_ABORT_STARTUP = -1;

int parseArguments(int argc, char* argv[], entt::registry& _Reg)
{
    argagg::parser ArgParser
        {{
            {"help", {"-h", "--help"},
             "Shows this help message", 0},
            {"port", {"-p", "--port"},
             "Port to listen to", 1}
        }};

    argagg::parser_results Args;
    try
    {
        Args = ArgParser.parse(argc, argv);
    }
    catch (const std::exception& e)
    {
        _Reg.ctx<MessageHandler>().report("prg", "Couldn't parse command line arguments, error: "+
                                           std::string(e.what()));
        return PWNG_ABORT_STARTUP;
    }
    if (Args["help"])
    {
        std::stringstream Message;
        Message << ArgParser;
        _Reg.ctx<MessageHandler>().report("prg", Message.str(), MessageHandler::INFO);
        return PWNG_ABORT_STARTUP;
    }

    int Port = 9002;
    if (Args["port"])
    {
        Port = Args["port"];
    }

    return Port;
}

int main(int argc, char* argv[])
{
    entt::registry Reg;

    Reg.set<MessageHandler>();
    Reg.set<NetworkManager>(Reg);
    Reg.set<SimulationManager>(Reg);

    auto& Messages = Reg.ctx<MessageHandler>();
    auto& Network = Reg.ctx<NetworkManager>();
    auto& Simulation = Reg.ctx<SimulationManager>();

    Messages.registerSource("net", "net");
    Messages.registerSource("prg", "prg");
    Messages.registerSource("sim", "sim");
    Messages.setColored(true);
    Messages.setLevel(MessageHandler::DEBUG_L3);

    int Port = parseArguments(argc, argv, Reg);
    if (Port != PWNG_ABORT_STARTUP)
    {
        moodycamel::ConcurrentQueue<NetworkMessage> InputQueue;
        moodycamel::ConcurrentQueue<NetworkMessage> OutputQueue;

        if (Network.init(&InputQueue, &OutputQueue, Port))
        {

            Simulation.init(&InputQueue, &OutputQueue);

            while (Network.isRunning() || Simulation.isRunning())

            {
                NetworkMessage Message;
                while (InputQueue.try_dequeue(Message))
                {
                    DBLK(Messages.report("prg", "Dequeueing incoming message:\n"+Message.Payload, MessageHandler::DEBUG_L3);)

                    json j = json::parse(Message.Payload);

                    if (j["params"]["Message"] == "get_data")
                    {
                        Messages.report("prg", "Static galaxy data requested", MessageHandler::INFO);
                        json Result =
                        {
                            {"jsonrpc", "2.0"},
                            {"result", "success"},
                            {"id", j["id"]}
                        };
                        OutputQueue.enqueue({Message.ID, Result.dump(4)});
                        Simulation.queueGalaxyData(Message.ID);
                    }
                    if (j["params"]["Message"] == "sub_server_stats")
                    {
                        Messages.report("prg", "Subscribe on server stats requested", MessageHandler::INFO);
                        Reg.emplace_or_replace<ServerStatusSubscriptionComponent>(Message.ID);
                    }
                    if (j["params"]["Message"] == "unsub_server_stats")
                    {
                        Messages.report("prg", "Unsubscribe on server stats requested", MessageHandler::INFO);
                        Reg.remove_if_exists<ServerStatusSubscriptionComponent>(Message.ID);
                    }
                    if (j["params"]["Message"] == "start_simulation")
                    {
                        Messages.report("prg", "Simulation start requested", MessageHandler::INFO);
                        json Result =
                        {
                            {"jsonrpc", "2.0"},
                            {"result", "success"},
                            {"id", j["id"]}
                        };
                        OutputQueue.enqueue({Message.ID, Result.dump(4)});
                        Simulation.start();
                    }
                    if (j["params"]["Message"] == "stop_simulation")
                    {
                        Messages.report("prg", "Simulation stop requested", MessageHandler::INFO);
                        json Result =
                        {
                            {"jsonrpc", "2.0"},
                            {"result", "success"},
                            {"id", j["id"]}
                        };
                        OutputQueue.enqueue({Message.ID, Result.dump(4)});
                        Simulation.stop();
                    }
                    if (j["params"]["Message"] == "shutdown")
                    {
                        Messages.report("prg", "Server shutdown requested", MessageHandler::INFO);
                        json Result =
                        {
                            {"jsonrpc", "2.0"},
                            {"result", "success"},
                            {"id", j["id"]}
                        };
                        Simulation.stop();
                        OutputQueue.enqueue({Message.ID, Result.dump(4)});
                        Messages.report("prg", "Shutting down...", MessageHandler::INFO);
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                        Network.stop();
                    }

                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

        }

        Messages.report("prg", "Exit program", MessageHandler::INFO);

        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_FAILURE;
    }
}
