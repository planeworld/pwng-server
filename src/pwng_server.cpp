#include <iostream>

#include <argagg/argagg.hpp>
#include <entt/entity/registry.hpp>
#include <concurrentqueue/concurrentqueue.h>

#include "network_manager.hpp"
#include "simulation_manager.hpp"

constexpr int PWNG_ABORT_STARTUP = -1;

int parseArguments(int argc, char* argv[])
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
        std::cerr << e.what() << '\n';
        return PWNG_ABORT_STARTUP;
    }
    if (Args["help"])
    {
        std::cerr << ArgParser;
        return PWNG_ABORT_STARTUP;
    }

    int Port = 9002;
    if (Args["port"])
    {
        Port = Args["port"];
    }

    std::cout << Port << std::endl;

    return Port;
}

int main(int argc, char* argv[])
{
    int Port = parseArguments(argc, argv);
    if (Port != PWNG_ABORT_STARTUP)
    {
        entt::registry Reg;
        moodycamel::ConcurrentQueue<std::string> InputQueue;
        moodycamel::ConcurrentQueue<std::string> OutputQueue;

        Reg.set<NetworkManager>();
        Reg.ctx<NetworkManager>().init(&InputQueue, &OutputQueue, Port);
        Reg.set<SimulationManager>();

        Reg.ctx<SimulationManager>().init();

        bool IsRunning = true;
        while (IsRunning)
        {
            std::string Message;
            bool NewMessageFound = InputQueue.try_dequeue(Message);
            if (NewMessageFound)
                std::cout << Message << std::endl;
        }
        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_FAILURE;
    }
}
