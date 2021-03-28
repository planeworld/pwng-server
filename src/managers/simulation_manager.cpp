#include "simulation_manager.hpp"

#include "message_handler.hpp"

#include "acceleration_component.hpp"
#include "body_component.hpp"
#include "position_component.hpp"
#include "velocity_component.hpp"

SimulationManager::~SimulationManager()
{
    auto view = Reg_.view<PositionComponent,
                          VelocityComponent,
                          AccelerationComponent,
                          BodyComponent>();
    Reg_.destroy(view.begin(), view.end());
}

void SimulationManager::init(moodycamel::ConcurrentQueue<std::string>* const _InputQueue,
                             moodycamel::ConcurrentQueue<std::string>* const _OutputQueue)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    Messages.report("sim", "Initialising Simulation Manager", MessageHandler::INFO);

    InputQueue_ = _InputQueue;
    OutputQueue_ = _OutputQueue;

    World_ = new b2World({0.0f, 0.0f});
    World2_ = new b2World({0.0f, 0.0f});

    auto GroupAV = Reg_.group<AccelerationComponent>(entt::get<VelocityComponent>);
    auto GroupVP = Reg_.group<VelocityComponent,PositionComponent>();
    auto GroupVPAB = Reg_.group<VelocityComponent,PositionComponent>(
                                entt::get<AccelerationComponent, BodyComponent>);

    // 149 598 022,96
    auto Earth = Reg_.create();
    Reg_.emplace<PositionComponent>(Earth, Vec2Dd{0.0, -152.1e9});
    // Reg_.emplace<PositionComponent>(Earth, Vec2Dd{0.0, -152.1e2});
    Reg_.emplace<VelocityComponent>(Earth, Vec2Dd{29.29e3, 0.0});
    Reg_.emplace<AccelerationComponent>(Earth, Vec2Dd{0.0, 0.0});
    Reg_.emplace<BodyComponent>(Earth, 5.972e24, 8.008e37);

    auto Sun = Reg_.create();
    Reg_.emplace<PositionComponent>(Sun, Vec2Dd{0.0, 0.0});
    Reg_.emplace<VelocityComponent>(Sun, Vec2Dd{0.0, 0.0});
    Reg_.emplace<AccelerationComponent>(Sun, Vec2Dd{0.0, 0.0});
    Reg_.emplace<BodyComponent>(Sun, 1.9884e30, 1.0);

    std::cout << GroupAV.size() << std::endl;

    this->start();
}

void SimulationManager::start()
{
    if (!IsRunning_)
    {
        auto& Messages = Reg_.ctx<MessageHandler>();

        IsRunning_ = true;
        Thread_ = std::thread(&SimulationManager::run, this);
        Messages.report("sim", "Simulation thread started successfully", MessageHandler::INFO);
    }
}

void SimulationManager::stop()
{
    if (IsRunning_)
    {
        auto& Messages = Reg_.ctx<MessageHandler>();

        IsRunning_ = false;
        Thread_.join();
        Messages.report("sim","Simulation stopped", MessageHandler::INFO);
    }
}

void SimulationManager::run()
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    Messages.report("sim", "Simulation Manager running", MessageHandler::INFO);

    while (IsRunning_)
    {
        // World_->Step(1.0f/60.0f, 8, 3);
        // World2_->Step(1.0f/60.0f, 8, 3);
        // World_->ShiftOrigin({20.0f, 10.0f});

        SysGravity_.calculateForces();
        SysIntegrator_.integrate(3600.0);

        static std::uint32_t c{0u};

        if (c == 10u)
        {
            Reg_.group<VelocityComponent,
                    PositionComponent>(entt::get<
                    AccelerationComponent,
                    BodyComponent>).each
                ([&](auto _e, const auto& _v, const auto& _p, const auto& _a, const auto& _b)
                {
                    json j =
                    {
                        {"jsonrpc", "2.0"},
                        {"method", "sim_broadcast"},
                        {"params",
                        {{"eid", std::uint32_t(_e)},
                        {"ts", "insert timestamp here"},
                        {"m", _b.m}, {"i", _b.i},
                        {"ax", _a.v(0)}, {"ay", _a.v(1)},
                        {"vx", _v.v(0)}, {"vy", _v.v(1)},
                        {"px", _p.v(0)}, {"py", _p.v(1)}}}
                    };
                    OutputQueue_->enqueue(j.dump(4));
                });
            c = 0u;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(SimStepSize_));
        ++c;
    }

    Messages.report("sim", "Simulation thread stopped successfully", MessageHandler::INFO);
}
