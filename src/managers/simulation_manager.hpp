#ifndef SIMULATION_MANAGER_HPP
#define SIMULATION_MANAGER_HPP

#include <chrono>
#include <memory>
#include <thread>

#include <box2d/box2d.h>
#include <concurrentqueue/concurrentqueue.h>
#include <entt/entity/registry.hpp>

#include "json_manager.hpp"
#include "gravity_system.hpp"
#include "integrator_system.hpp"
#include "name_system.hpp"
#include "network_message.hpp"
#include "sim_timer.hpp"
#include "timer.hpp"

class SimulationManager
{

    public:

        explicit SimulationManager(entt::registry& _Reg) : Reg_(_Reg),
                                                           SysGravity_(_Reg),
                                                           SysIntegrator_(_Reg),
                                                           SysName_(_Reg){}
        ~SimulationManager();

        bool isRunning() const {return IsRunning_;}

        void init(moodycamel::ConcurrentQueue<NetworkMessage>* const _QueueSimIn,
                  moodycamel::ConcurrentQueue<NetworkMessage>* const _OutputQueue);


    private:

        std::uint64_t getTimeStamp() const;

        void queueDynamicData(entt::entity _ClientID) const;
        void queueGalaxyData(entt::entity _ClientID, JsonManager::RequestIDType _ReqID) const;
        void queueServerStats(entt::entity _ClientID);
        void queueTireData(entt::entity _ClientID) const;
        void run();
        void shutdown();
        void start();
        void stop();

        void createTire();

        entt::registry&  Reg_;
        GravitySystem    SysGravity_;
        IntegratorSystem SysIntegrator_;
        NameSystem       SysName_;

        moodycamel::ConcurrentQueue<NetworkMessage>* QueueSimIn_{nullptr};
        moodycamel::ConcurrentQueue<NetworkMessage>* OutputQueue_{nullptr};

        SimTimer SimTime_;
        Timer QueueInTimer_;
        Timer QueueOutTimer_;
        Timer PhysicsTimer_;
        Timer SimulationTimer_;
        double QueueOutTime_{0.0};
        double SimulationTime_{0.0};

        std::uint32_t SimStepSize_{10};

        b2World*    World_{nullptr};
        std::thread Thread_;

        bool IsRunning_{false};
        bool IsSimRunning_{false};
};

#endif // SIMULATION_MANAGER_HPP
