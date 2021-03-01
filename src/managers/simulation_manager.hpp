#ifndef SIMULATION_MANAGER_HPP
#define SIMULATION_MANAGER_HPP

#include <chrono>
#include <memory>
#include <thread>
#include <box2d/box2d.h>
#include <concurrentqueue/concurrentqueue.h>

class SimulationManager
{

    public:

        void init()
        {
            Thread_ = std::thread(&SimulationManager::run, this);
            World_ = new b2World({0.0f, 0.0f});
            World2_ = new b2World({0.0f, 0.0f});
            std::cout << "Created Worlds" << std::endl;
        }

    private:

        void run()
        {
            while (!ExitSimulation_)
            {
                // std::cout << "[Thread] Simulation running" << std::endl;
                World_->Step(1.0f/60.0f, 8, 3);
                World2_->Step(1.0f/60.0f, 8, 3);
                // World_->ShiftOrigin({20.0f, 10.0f});
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
            std::cout << "[Thread] Simulation exited." << std::endl;
        }

        b2World*    World_;
        b2World*    World2_;
        std::thread Thread_;
        bool        ExitSimulation_{false};
};

#endif // SIMULATION_MANAGER_HPP
