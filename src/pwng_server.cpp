#include <iostream>
#include <sstream>

#include <argagg/argagg.hpp>
#include <concurrentqueue/concurrentqueue.h>
#include <entt/entity/registry.hpp>
#include <rapidjson/document.h>

#include "json_manager.hpp"
#include "message_handler.hpp"
#include "network_manager.hpp"
#include "position_component.hpp"
#include "simulation_manager.hpp"
#include "subscription_components.hpp"
#include "velocity_component.hpp"

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
    using namespace rapidjson;

    entt::registry Reg;

    Reg.set<MessageHandler>();

    auto& Messages = Reg.ctx<MessageHandler>();

    Messages.registerSource("net", "net");
    Messages.registerSource("prg", "prg");
    Messages.registerSource("sim", "sim");
    Messages.registerSource("jsn", "jsn");
    Messages.setColored(true);
    Messages.setLevel(MessageHandler::DEBUG_L3);

    int Port = parseArguments(argc, argv, Reg);
    if (Port != PWNG_ABORT_STARTUP)
    {
        moodycamel::ConcurrentQueue<NetworkMessage> InputQueue;
        moodycamel::ConcurrentQueue<NetworkMessage> OutputQueue;
        moodycamel::ConcurrentQueue<NetworkMessage> QueueSimIn;

        Reg.set<JsonManager>(Reg);
        Reg.set<NetworkManager>(Reg);
        Reg.set<SimulationManager>(Reg);
        auto& Network = Reg.ctx<NetworkManager>();
        auto& Simulation = Reg.ctx<SimulationManager>();

        if (Network.init(&InputQueue, &OutputQueue, Port))
        {
            Timer MainTimer;

            Simulation.init(&QueueSimIn, &OutputQueue);

            while (Network.isRunning() || Simulation.isRunning())
            {
                MainTimer.start();

                NetworkMessage Message;
                while (InputQueue.try_dequeue(Message))
                {
                    DBLK(Messages.report("prg", "Dequeueing incoming message:\n"+Message.Payload, MessageHandler::DEBUG_L3);)

                    Document d;
                    d.Parse(Message.Payload.c_str());
                    auto& Command = d["method"];

                    // Server addressed commands
                    if (Command == "sub_server_stats")
                    {
                        Messages.report("prg", "Subscribe on server stats requested", MessageHandler::INFO);
                        Reg.emplace_or_replace<ServerStatusSubscriptionComponent>(Message.ClientID);
                    }
                    else if (Command == "unsub_server_stats")
                    {
                        Messages.report("prg", "Unsubscribe on server stats requested", MessageHandler::INFO);
                        Reg.remove_if_exists<ServerStatusSubscriptionComponent>(Message.ClientID);
                    }
                    // Simulation addressed commands
                    else if (Command == "get_data")
                    {
                        Messages.report("prg", "Static galaxy data requested", MessageHandler::INFO);
                        DBLK(Messages.report("prg", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
                        QueueSimIn.enqueue(Message);
                    }
                    else if (Command == "shutdown")
                    {
                        Messages.report("prg", "Server shutdown requested", MessageHandler::INFO);
                        Messages.report("prg", "Shutting down...", MessageHandler::INFO);
                        DBLK(Messages.report("prg", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
                        QueueSimIn.enqueue(Message);
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                        Network.stop();
                    }
                    else if (Command == "start_simulation")
                    {
                        Messages.report("prg", "Simulation start requested", MessageHandler::INFO);
                        DBLK(Messages.report("prg", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
                        QueueSimIn.enqueue(Message);
                    }
                    else if (Command == "stop_simulation")
                    {
                        Messages.report("prg", "Simulation stop requested", MessageHandler::INFO);
                        DBLK(Messages.report("prg", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
                        QueueSimIn.enqueue(Message);
                    }
                    else if (Command == "sub_dynamic_data")
                    {
                        Messages.report("prg", "Subscribe on dynamic data requested", MessageHandler::INFO);
                        DBLK(Messages.report("prg", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
                        QueueSimIn.enqueue(Message);
                    }
                    else if (Command == "sub_system")
                    {
                        Messages.report("prg", "Subscribe on star system requested", MessageHandler::INFO);
                        DBLK(Messages.report("prg", "Appending request to simulation queue", MessageHandler::DEBUG_L1);)
                        QueueSimIn.enqueue(Message);
                    }

                }
                MainTimer.stop();
                if (10 - MainTimer.elapsed_ms() > 0.0)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10 - int(MainTimer.elapsed_ms())));
                }
                else
                {
                    Messages.report("prg", "Thread processing exceeds step time ("
                                            + std::to_string(MainTimer.elapsed_ms())
                                            +"/10.0)ms", MessageHandler::WARNING);
                }
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
