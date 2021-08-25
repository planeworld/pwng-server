#include <iostream>
#include <sstream>

#include <argagg/argagg.hpp>
#include <concurrentqueue/concurrentqueue.h>
#include <entt/entity/registry.hpp>
#include <rapidjson/document.h>

#include "json_manager.hpp"
#include "message_handler.hpp"
#include "network_manager.hpp"
#include "network_message_broker.hpp"
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
    Messages.registerSource("brk", "brk");
    Messages.setColored(true);
    Messages.setLevel(MessageHandler::DEBUG_L3);

    int Port = parseArguments(argc, argv, Reg);
    if (Port != PWNG_ABORT_STARTUP)
    {
        moodycamel::ConcurrentQueue<NetworkMessage> InputQueue;
        moodycamel::ConcurrentQueue<NetworkMessage> OutputQueue;
        moodycamel::ConcurrentQueue<NetworkDocument> QueueSimIn;
        moodycamel::ConcurrentQueue<NetworkDocument> QueueNetIn;

        Reg.set<JsonManager>(Reg);
        Reg.set<NetworkManager>(Reg);
        Reg.set<NetworkMessageBroker>(Reg, &QueueSimIn, &QueueNetIn);
        Reg.set<SimulationManager>(Reg);
        auto& Broker = Reg.ctx<NetworkMessageBroker>();
        auto& Network = Reg.ctx<NetworkManager>();
        auto& Simulation = Reg.ctx<SimulationManager>();

        if (Network.init(&QueueNetIn, &InputQueue, &OutputQueue, Port))
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

                    // const auto d = Broker.parse(Message);
                    Broker.process(Broker.parse(Message));
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
