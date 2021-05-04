#ifndef SIMULATION_MANAGER_HPP
#define SIMULATION_MANAGER_HPP

#include <chrono>
#include <memory>
#include <thread>
#include <box2d/box2d.h>
#include <concurrentqueue/concurrentqueue.h>
#include <entt/entity/registry.hpp>

#include "gravity_system.hpp"
#include "integrator_system.hpp"
#include "name_system.hpp"
#include "network_message.hpp"
#include "timer.hpp"

class SimulationManager
{

    public:

        SimulationManager(entt::registry& _Reg) : Reg_(_Reg),
                                                  SysGravity_(_Reg),
                                                  SysIntegrator_(_Reg),
                                                  SysName_(_Reg){}
        ~SimulationManager();

        bool isRunning() const {return IsRunning_;}

        void init(moodycamel::ConcurrentQueue<NetworkMessage>* const _QueueSimIn,
                  moodycamel::ConcurrentQueue<NetworkMessage>* const _OutputQueue);


    private:

        void queueDynamicData(entt::entity _ID) const;
        void queueGalaxyData(entt::entity _ID) const;
        void queueServerStats(entt::entity _ID);
        void run();
        void shutdown();
        void start();
        void stop();

        entt::registry&  Reg_;
        GravitySystem    SysGravity_;
        IntegratorSystem SysIntegrator_;
        NameSystem       SysName_;

        moodycamel::ConcurrentQueue<NetworkMessage>* QueueSimIn_{nullptr};
        moodycamel::ConcurrentQueue<NetworkMessage>* OutputQueue_{nullptr};

        Timer QueueInTimer_;
        Timer QueueOutTimer_;
        Timer PhysicsTimer_;
        Timer SimulationTimer_;
        double QueueOutTime_{0.0};
        double SimulationTime_{0.0};

        std::uint32_t SimStepSize_{10};

        b2World*    World_;
        b2World*    World2_;
        std::thread Thread_;

        bool IsRunning_{false};
        bool IsSimRunning_{false};
};

#endif // SIMULATION_MANAGER_HPP
