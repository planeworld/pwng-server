#ifndef SIMULATION_MANAGER_HPP
#define SIMULATION_MANAGER_HPP

#include <chrono>
#include <memory>
#include <thread>
#include <concurrentqueue/concurrentqueue.h>

class SimulationManager
{

    public:

        void init()
        {
            Thread_ = std::thread(&SimulationManager::run, this);
        }

    private:

        void run()
        {
            while (!ExitSimulation_)
            {
                std::cout << "[Thread] Simulation running" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            std::cout << "[Thread] Simulation exited." << std::endl;
        }

        std::thread Thread_;
        bool        ExitSimulation_{false};
};

#endif // SIMULATION_MANAGER_HPP
