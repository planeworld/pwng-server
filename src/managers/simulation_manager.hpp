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

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class SimulationManager
{

    public:

        SimulationManager(entt::registry& _Reg) : Reg_(_Reg), SysIntegrator_(_Reg) {}
        ~SimulationManager();

        bool isRunning() const {return IsRunning_;}

        void init(moodycamel::ConcurrentQueue<std::string>* const _InputQueue,
                  moodycamel::ConcurrentQueue<std::string>* const _OutputQueue);

        void start();
        void stop();

    private:

        void run();

        entt::registry& Reg_;
        IntegratorSystem SysIntegrator_;

        moodycamel::ConcurrentQueue<std::string>* InputQueue_;
        moodycamel::ConcurrentQueue<std::string>* OutputQueue_;

        b2World*    World_;
        b2World*    World2_;
        std::thread Thread_;

        bool IsRunning_{false};
};

#endif // SIMULATION_MANAGER_HPP
