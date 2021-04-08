#include "simulation_manager.hpp"



#include <random>

#include "message_handler.hpp"

#include "acceleration_component.hpp"
#include "body_component.hpp"
#include "name_component.hpp"
#include "position_component.hpp"
#include "radius_component.hpp"
#include "star_definitions.hpp"
#include "timer.hpp"
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

    auto Earth = Reg_.create();
    Reg_.emplace<PositionComponent>(Earth, Vec2Dd{0.0, -152.1e9});
    Reg_.emplace<VelocityComponent>(Earth, Vec2Dd{29.29e3, 0.0});
    Reg_.emplace<AccelerationComponent>(Earth, Vec2Dd{0.0, 0.0});
    Reg_.emplace<BodyComponent>(Earth, 5.972e24, 8.008e37);
    Reg_.emplace<NameComponent>(Earth, "Earth");
    Reg_.emplace<RadiusComponent>(Earth, 6378137.0);

    auto Moon = Reg_.create();
    Reg_.emplace<PositionComponent>(Moon, Vec2Dd{384400.0e3, -152.1e9});
    Reg_.emplace<VelocityComponent>(Moon, Vec2Dd{29.29e3, 964.0});
    Reg_.emplace<AccelerationComponent>(Moon, Vec2Dd{0.0, 0.0});
    Reg_.emplace<BodyComponent>(Moon, 7.346e22, 1.0);
    Reg_.emplace<NameComponent>(Moon, "Moon");
    Reg_.emplace<RadiusComponent>(Moon, 1737.0e3);

    auto Sun = Reg_.create();
    Reg_.emplace<PositionComponent>(Sun, Vec2Dd{0.0, 0.0});
    Reg_.emplace<VelocityComponent>(Sun, Vec2Dd{0.0, 0.0});
    Reg_.emplace<AccelerationComponent>(Sun, Vec2Dd{0.0, 0.0});
    Reg_.emplace<BodyComponent>(Sun, 1.9884e30, 1.0);
    Reg_.emplace<NameComponent>(Sun, "Sun");
    Reg_.emplace<RadiusComponent>(Sun, 6.96342e8);

    std::mt19937 Generator;
    std::normal_distribution<double> DistMass(1.0e30, 0.5e30);
    std::normal_distribution<double> DistRadius(1.0e9, 1.0e9);
    std::normal_distribution<double> DistGalaxyArmScatter(0.0, 1.0);
    std::normal_distribution<double> DistGalaxyCenter(0.0, 0.5);
    std::poisson_distribution<int> DistGalaxyArms(3);
    std::normal_distribution<double> DistSpectralClass(0.0, 1.0);

    int Arms=DistGalaxyArms(Generator);
    if (Arms < 2) Arms = 2;
    Messages.report("sim", "Creating galaxy with " + std::to_string(Arms) + " spiral arms", MessageHandler::INFO);
    double Alpha = 30.0e13;
    double Scatter = 0.1; // 10%
    // double GalaxyRadiusMax = Alpha/(0.5*MATH_PI);
    double GalaxyPhiMin = 0.2 * MATH_PI;
    double GalaxyPhiMax = 4.0 * MATH_PI;
    double GalaxyRadiusMax = Alpha / GalaxyPhiMin;

    Arms = 2;

    int c{0};
    for (auto i=0; i<Arms; ++i)
    {
        for (auto Phi=GalaxyPhiMin; Phi<GalaxyPhiMax; Phi+=0.005)
        {
            auto e = Reg_.create();

            double r=Alpha/Phi;
            double p = Phi+2.0*MATH_PI/Arms*i;
            // double r_n = ((r/GalaxyRadiusMax)+1.0-Phi/GalaxyPhiMax)/2.0;
            // double r_n = r/GalaxyRadiusMax;
            double r_n = 1.0-Phi/GalaxyPhiMax;
            DistSpectralClass = std::normal_distribution<double>(r_n, 0.16);
            int SpectralClass = DistSpectralClass(Generator)*6;
            if (SpectralClass < 0) SpectralClass = 0;
            if (SpectralClass > 6) SpectralClass = 6;
            std::cout << SpectralClass << " ";

            Reg_.emplace<PositionComponent>(e, Vec2Dd{r*std::cos(p)+DistGalaxyArmScatter(Generator)*r*Scatter,
                                                      r*std::sin(p)+DistGalaxyArmScatter(Generator)*r*Scatter});
            Reg_.emplace<VelocityComponent>(e, Vec2Dd{0.0, 0.0});
            Reg_.emplace<AccelerationComponent>(e, Vec2Dd{0.0, 0.0});
            Reg_.emplace<BodyComponent>(e, StarMassDistribution[SpectralClass](Generator), 1.0);
            Reg_.emplace<StarDataComponent>(e, SpectralClassE(SpectralClass), StarTemperatureDistribution[SpectralClass](Generator));
            Reg_.emplace<NameComponent>(e, "Star_"+std::to_string(c));
            Reg_.emplace<RadiusComponent>(e, StarRadiusDistribution[SpectralClass](Generator));
            ++c;
        }
    }
    std::cout << std::endl;
    for (auto Phi=0.0; Phi<2.0*MATH_PI; Phi+=0.001)
    {
        auto e = Reg_.create();

        double r=std::abs(DistGalaxyCenter(Generator));

        DistSpectralClass = std::normal_distribution<double>(r, 0.16);
        int SpectralClass = DistSpectralClass(Generator)*6;
        if (SpectralClass < 0) SpectralClass = 0;
        if (SpectralClass > 6) SpectralClass = 6;
        std::cout << SpectralClass << " ";

        Reg_.emplace<PositionComponent>(e, 12.0e13*r*Vec2Dd{std::cos(Phi),std::sin(Phi)});
        Reg_.emplace<VelocityComponent>(e, Vec2Dd{0.0, 0.0});
        Reg_.emplace<AccelerationComponent>(e, Vec2Dd{0.0, 0.0});
        Reg_.emplace<BodyComponent>(e, StarMassDistribution[SpectralClass](Generator), 1.0);
        Reg_.emplace<NameComponent>(e, "Star_"+std::to_string(c));
        Reg_.emplace<StarDataComponent>(e, SpectralClassE(SpectralClass), StarTemperatureDistribution[SpectralClass](Generator));
        Reg_.emplace<RadiusComponent>(e, StarRadiusDistribution[SpectralClass](Generator));
        ++c;
    }
    Messages.report("sim", std::to_string(c) + " star systems generated", MessageHandler::INFO);
    std::cout << std::endl;

    // this->start();
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
    Timer SimulationTimer;

    Messages.report("sim", "Simulation Manager running", MessageHandler::INFO);

    while (IsRunning_)
    {
        SimulationTimer.start();

        // World_->Step(1.0f/60.0f, 8, 3);
        // World2_->Step(1.0f/60.0f, 8, 3);
        // World_->ShiftOrigin({20.0f, 10.0f});

        SysGravity_.calculateForces();
        SysIntegrator_.integrate(3600.0);

        static std::uint32_t c{0u};

        if (c == 2u)
        {
            Reg_.group<VelocityComponent,
                    PositionComponent>(entt::get<
                    AccelerationComponent,
                    BodyComponent,
                    RadiusComponent,
                    StarDataComponent,
                    NameComponent>).each
                ([&](auto _e, const auto& _v, const auto& _p, const auto& _a, const auto& _b, const auto& _r, const auto& _s, const auto& _n)
                {
                    json j =
                    {
                        {"jsonrpc", "2.0"},
                        {"method", "sim_broadcast"},
                        {"params",
                        {{"eid", std::uint32_t(_e)},
                        {"ts", "insert timestamp here"},
                        {"name", _n.n},
                        {"m", _b.m},
                        {"i", _b.i},
                        {"r", _r.r},
                        {"sc", _s.SpectralClass},
                        {"t", _s.Temperature},
                        {"ax", _a.v(0)}, {"ay", _a.v(1)},
                        {"vx", _v.v(0)}, {"vy", _v.v(1)},
                        {"px", _p.v(0)}, {"py", _p.v(1)}}}
                    };
                    OutputQueue_->enqueue(j.dump(4));
                });
            c = 0u;
        }

        SimulationTimer.stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(SimStepSize_));
        ++c;
    }

    Messages.report("sim", "Simulation thread stopped successfully", MessageHandler::INFO);
}
