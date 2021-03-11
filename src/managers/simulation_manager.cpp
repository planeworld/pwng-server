#include "simulation_manager.hpp"

#include <iostream>

#include "message_handler.hpp"

void SimulationManager::init(moodycamel::ConcurrentQueue<std::string>* const _InputQueue,
                             moodycamel::ConcurrentQueue<std::string>* const _OutputQueue)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    Messages.report("SIM", "Initialising Simulation Manager");

    InputQueue_ = _InputQueue;
    OutputQueue_ = _OutputQueue;

    World_ = new b2World({0.0f, 0.0f});
    World2_ = new b2World({0.0f, 0.0f});
    auto e = Reg_.create();
    Reg_.emplace<PositionComponent<double>>(e);
    Reg_.emplace<VelocityComponent<double>>(e);
    Reg_.emplace<AccelerationComponent<double>>(e, 1.0, 0.5);
    Thread_ = std::thread(&SimulationManager::run, this);
}

void SimulationManager::stop()
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    auto view = Reg_.view<PositionComponent<double>,
                          VelocityComponent<double>,
                          AccelerationComponent<double>>();
    Reg_.destroy(view.begin(), view.end());

    IsRunning_ = false;
    Thread_.join();
    Messages.report("SIM","Simulation stopped");
}

void SimulationManager::run()
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    Messages.report("SIM", "Simulation Manager running");

    while (IsRunning_)
    {
        World_->Step(1.0f/60.0f, 8, 3);
        World2_->Step(1.0f/60.0f, 8, 3);
        // World_->ShiftOrigin({20.0f, 10.0f});

        SysIntegrator_.integrate(1.0/60.0);

        Reg_.group<PositionComponent<double>,
                    VelocityComponent<double>,
                    AccelerationComponent<double>>().each
            ([&](auto _e, auto& _p, auto& _v, auto& _a)
            {
                json j =
                {
                    {"id", std::uint32_t(_e)},
                    {"ax", _a.x}, {"ay", _a.y},
                    {"vx", _v.x}, {"vy", _v.y},
                    {"px", _p.x}, {"py", _p.y},
                };
                OutputQueue_->enqueue(j.dump(4));
            });

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    Messages.report("SIM", "Simulation thread stopped successfully");
}
