#ifndef SIMULATION_MANAGER_HPP
#define SIMULATION_MANAGER_HPP

#include <chrono>
#include <memory>
#include <thread>
#include <box2d/box2d.h>
#include <concurrentqueue/concurrentqueue.h>
#include <entt/entity/registry.hpp>

#include "acceleration_component.hpp"
#include "position_component.hpp"
#include "velocity_component.hpp"
#include "integrator_system.hpp"

class SimulationManager
{

    public:

        SimulationManager(entt::registry& _Reg) : Reg_(_Reg), SysIntegrator_(_Reg) {}

        void init()
        {
            World_ = new b2World({0.0f, 0.0f});
            World2_ = new b2World({0.0f, 0.0f});
            std::cout << "Created Worlds" << std::endl;
            e = Reg_.create();
            Reg_.emplace<PositionComponent<double>>(e);
            Reg_.emplace<VelocityComponent<double>>(e);
            Reg_.emplace<AccelerationComponent<double>>(e, 1.0, 0.5);
            Thread_ = std::thread(&SimulationManager::run, this);
        }

    private:

        void run()
        {
            while (!ExitSimulation_)
            {
                World_->Step(1.0f/60.0f, 8, 3);
                World2_->Step(1.0f/60.0f, 8, 3);
                // World_->ShiftOrigin({20.0f, 10.0f});

                SysIntegrator_.integrate(1.0/60.0);

                auto& c = Reg_.get<PositionComponent<double>>(e);
                std::cout << "Entity: " << std::uint32_t(e) << "; " << c.x << ", " << c.y << std::endl;

                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
            std::cout << "[Thread] Simulation exited." << std::endl;
        }

        entt::registry& Reg_;
        IntegratorSystem SysIntegrator_;

        entt::entity e;

        b2World*    World_;
        b2World*    World2_;
        std::thread Thread_;
        bool        ExitSimulation_{false};
};

#endif // SIMULATION_MANAGER_HPP
