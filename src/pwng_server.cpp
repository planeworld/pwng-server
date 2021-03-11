#include <iostream>
#include <sstream>

#include <argagg/argagg.hpp>
#include <concurrentqueue/concurrentqueue.h>
#include <entt/entity/registry.hpp>
#include <nlohmann/json.hpp>

#include "error_handler.hpp"
#include "message_handler.hpp"
#include "network_manager.hpp"
#include "position_component.hpp"
#include "simulation_manager.hpp"
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
        _Reg.ctx<ErrorHandler>().report("PRG", "Couldn't parse command line arguments, error: "+
                                        std::string(e.what()));
        return PWNG_ABORT_STARTUP;
    }
    if (Args["help"])
    {
        std::stringstream Message;
        Message << ArgParser;
        _Reg.ctx<MessageHandler>().report("PRG", Message.str(), MessageHandler::INFO);
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

    Reg.set<ErrorHandler>();
    Reg.set<MessageHandler>();
    Reg.set<NetworkManager>(Reg);
    Reg.set<SimulationManager>(Reg);

    auto& Errors = Reg.ctx<ErrorHandler>();
    auto& Messages = Reg.ctx<MessageHandler>();
    auto& Network = Reg.ctx<NetworkManager>();
    auto& Simulation = Reg.ctx<SimulationManager>();

    Messages.setLevel(MessageHandler::DEBUG_L3);

    int Port = parseArguments(argc, argv, Reg);
    if (Port != PWNG_ABORT_STARTUP)
    {

        auto IntegratorGroup = Reg.group<PositionComponent<double>,
                                         VelocityComponent<double>,
                                         AccelerationComponent<double>>();

        moodycamel::ConcurrentQueue<std::string> InputQueue;
        moodycamel::ConcurrentQueue<std::string> OutputQueue;

        Network.init(&InputQueue, &OutputQueue, Port);
        Simulation.init(&InputQueue, &OutputQueue);

        while (Network.isRunning() || Simulation.isRunning())

        {
            std::string Message;
            bool NewMessageFound = InputQueue.try_dequeue(Message);
            if (NewMessageFound)
            {
                DBLK(Messages.report("PRG", "Incoming Message: \n" + Message, MessageHandler::DEBUG_L1);)

                json j = json::parse(Message);

                if (j["Message"] == "stop")
                {
                    Network.stop();
                    Simulation.stop();
                }

            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        Messages.report("PRG", "Exit program");

        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_FAILURE;
    }
}
