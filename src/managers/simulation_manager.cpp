#include "simulation_manager.hpp"

#include <random>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "message_handler.hpp"

#include "acceleration_component.hpp"
#include "body_component.hpp"
#include "name_component.hpp"
#include "position_component.hpp"
#include "radius_component.hpp"
#include "star_definitions.hpp"
#include "sim_components.hpp"
#include "subscription_components.hpp"
#include "velocity_component.hpp"


SimulationManager::~SimulationManager()
{
    if (Thread_.joinable()) Thread_.join();
}

void SimulationManager::init(moodycamel::ConcurrentQueue<NetworkMessage>* const _QueueSimIn,
                             moodycamel::ConcurrentQueue<NetworkMessage>* const _OutputQueue)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    Messages.report("sim", "Initialising Simulation Manager", MessageHandler::INFO);

    QueueSimIn_ = _QueueSimIn;
    OutputQueue_ = _OutputQueue;

    World_ = new b2World({0.0f, 0.0f});
    World2_ = new b2World({0.0f, 0.0f});

    auto GroupAV = Reg_.group<AccelerationComponent>(entt::get<VelocityComponent>);
    auto GroupVP = Reg_.group<VelocityComponent,PositionComponent>();
    auto GroupVPAB = Reg_.group<VelocityComponent,PositionComponent>(
                                entt::get<AccelerationComponent, BodyComponent>);

    Vec2Dd SolarSystemPosition{0.0, 6.0e21};

    auto Earth = Reg_.create();
    Reg_.emplace<SystemPositionComponent>(Earth, SolarSystemPosition);
    Reg_.emplace<PositionComponent>(Earth, Vec2Dd{0.0, -152.1e9});
    Reg_.emplace<VelocityComponent>(Earth, Vec2Dd{29.29e3, 0.0});
    Reg_.emplace<AccelerationComponent>(Earth, Vec2Dd{0.0, 0.0});
    Reg_.emplace<BodyComponent>(Earth, 5.972e24, 8.008e37);
    Reg_.emplace<NameComponent>(Earth, "Earth");
    Reg_.emplace<RadiusComponent>(Earth, 6378137.0);

    auto Moon = Reg_.create();
    Reg_.emplace<SystemPositionComponent>(Moon, SolarSystemPosition);
    Reg_.emplace<PositionComponent>(Moon, Vec2Dd{384400.0e3, -152.1e9});
    Reg_.emplace<VelocityComponent>(Moon, Vec2Dd{29.29e3, 964.0});
    Reg_.emplace<AccelerationComponent>(Moon, Vec2Dd{0.0, 0.0});
    Reg_.emplace<BodyComponent>(Moon, 7.346e22, 1.0);
    Reg_.emplace<NameComponent>(Moon, "Moon");
    Reg_.emplace<RadiusComponent>(Moon, 1737.0e3);

    auto Sun = Reg_.create();
    Reg_.emplace<SystemPositionComponent>(Sun, SolarSystemPosition);
    Reg_.emplace<PositionComponent>(Sun, Vec2Dd{0.0, 0.0});
    Reg_.emplace<VelocityComponent>(Sun, Vec2Dd{0.0, 0.0});
    Reg_.emplace<AccelerationComponent>(Sun, Vec2Dd{0.0, 0.0});
    Reg_.emplace<BodyComponent>(Sun, 1.9884e30, 1.0);
    Reg_.emplace<NameComponent>(Sun, "Sun");
    Reg_.emplace<StarDataComponent>(Sun, SpectralClassE::G, 5778.0);
    Reg_.emplace<RadiusComponent>(Sun, 6.96342e8);

    auto SolarSystem = Reg_.create();
    auto& SolarSystemObjects = Reg_.emplace<StarSystemComponent>(SolarSystem);
    SolarSystemObjects.Objects = {Sun, Earth, Moon};

    std::mt19937 Generator;
    std::normal_distribution<double> DistGalaxyArmScatter(0.0, 1.0);
    std::normal_distribution<double> DistGalaxyCenter(0.0, 0.5);
    std::poisson_distribution<int> DistGalaxyArms(3);
    std::normal_distribution<double> DistSpectralClass(0.0, 1.0);

    int Arms=DistGalaxyArms(Generator);
    if (Arms < 2) Arms = 2;
    Messages.report("sim", "Creating galaxy with " + std::to_string(Arms) + " spiral arms", MessageHandler::INFO);
    double Alpha = 1.0e22;
    double Scatter = 0.1; // 10%
    double GalaxyPhiMin = 0.2 * MATH_PI;
    double GalaxyPhiMax = 4.0 * MATH_PI;
    double GalaxyRadiusMax = Alpha / GalaxyPhiMin;
    double GalaxyDistanceMean = 5.0e16;

    Arms = 2;

    DBLK(Messages.report("sim", "Creating spiral arms", MessageHandler::DEBUG_L1);)
    DBLK(Messages.report("sim", "Distribution of spectral classes (0-6 = M-O):", MessageHandler::DEBUG_L3);)

    int c{0};
    for (auto i=0; i<Arms; ++i)
    {
        for (auto Phi=GalaxyPhiMin; Phi<GalaxyPhiMax; Phi+=0.005)
        {
            auto e = Reg_.create();

            double r=Alpha/Phi;
            double p = Phi+2.0*MATH_PI/Arms*i;
            double r_n = 1.0-Phi/GalaxyPhiMax;
            DistSpectralClass = std::normal_distribution<double>(r_n, 0.16);
            int SpectralClass = DistSpectralClass(Generator)*6;
            if (SpectralClass < 0) SpectralClass = 0;
            if (SpectralClass > 6) SpectralClass = 6;
            DBLK(std::cout << SpectralClass << " ";)

            Reg_.emplace<SystemPositionComponent>(e, Vec2Dd{r*std::cos(p)+DistGalaxyArmScatter(Generator)*r*Scatter,
                                                            r*std::sin(p)+DistGalaxyArmScatter(Generator)*r*Scatter});
            Reg_.emplace<BodyComponent>(e, StarMassDistribution[SpectralClass](Generator), 1.0);
            Reg_.emplace<StarDataComponent>(e, SpectralClassE(SpectralClass), StarTemperatureDistribution[SpectralClass](Generator));
            Reg_.emplace<NameComponent>(e, "Star_"+std::to_string(c));
            Reg_.emplace<RadiusComponent>(e, StarRadiusDistribution[SpectralClass](Generator));
            ++c;
        }
    }
    DBLK(std::cout << std::endl;)
    DBLK(Messages.report("sim", "Creating center", MessageHandler::DEBUG_L1);)
    DBLK(Messages.report("sim", "Distribution of spectral classes (0-6 = M-O):", MessageHandler::DEBUG_L3);)
    for (auto Phi=0.0; Phi<2.0*MATH_PI; Phi+=0.001)
    {
        auto e = Reg_.create();

        double r=std::abs(DistGalaxyCenter(Generator));

        DistSpectralClass = std::normal_distribution<double>(r, 0.16);
        int SpectralClass = DistSpectralClass(Generator)*6;
        if (SpectralClass < 0) SpectralClass = 0;
        if (SpectralClass > 6) SpectralClass = 6;
        DBLK(std::cout << SpectralClass << " ";)

        Reg_.emplace<SystemPositionComponent>(e, 0.5e22*r*Vec2Dd{std::cos(Phi),std::sin(Phi)});
        Reg_.emplace<BodyComponent>(e, StarMassDistribution[SpectralClass](Generator), 1.0);
        Reg_.emplace<NameComponent>(e, "Star_"+std::to_string(c));
        Reg_.emplace<StarDataComponent>(e, SpectralClassE(SpectralClass), StarTemperatureDistribution[SpectralClass](Generator));
        Reg_.emplace<RadiusComponent>(e, StarRadiusDistribution[SpectralClass](Generator));
        ++c;
    }
    DBLK(std::cout << std::endl;)
    Messages.report("sim", std::to_string(c) + " star systems generated", MessageHandler::INFO);

    Thread_ = std::thread(&SimulationManager::run, this);
    Messages.report("sim", "Simulation thread started successfully", MessageHandler::INFO);

}

void SimulationManager::queueDynamicData(entt::entity _ID) const
{
    using namespace rapidjson;

    Reg_.view<BodyComponent,
              NameComponent,
              PositionComponent,
              RadiusComponent,
              SystemPositionComponent>().each
        ([&](auto _e, const auto& _b, const auto& _n, const auto& _p,
                      const auto& _r, const auto& _s)
        {
            StringBuffer s;
            Writer<StringBuffer> w(s);

            w.StartObject();
            w.Key("jsonrpc"); w.String("2.0");
            w.Key("method"); w.String("bc_dynamic_data");
            w.Key("params");
                w.StartObject();
                w.Key("eid"); w.Uint(entt::to_integral(_e));
                w.Key("ts"); w.String("insert timestamp here");
                w.Key("name"); w.String(_n.n.c_str());
                w.Key("m"); w.Double(_b.m);
                w.Key("i"); w.Double(_b.i);
                w.Key("r"); w.Double(_r.r);
                w.Key("spx"); w.Double(_s.v(0)); w.Key("spy"); w.Double(_s.v(1));
                w.Key("px"); w.Double(_p.v(0)); w.Key("py"); w.Double(_p.v(1));
                w.EndObject();
            w.EndObject();
            OutputQueue_->enqueue({_ID, s.GetString()});
        });
}

void SimulationManager::queueGalaxyData(entt::entity _ID) const
{
    using namespace rapidjson;

    Reg_.view<SystemPositionComponent,
              BodyComponent,
              RadiusComponent,
              StarDataComponent,
              NameComponent>().each
        ([&](auto _e, const auto& _p, const auto& _b, const auto& _r, const auto& _s, const auto& _n)
        {
            StringBuffer s;
            Writer<StringBuffer> w(s);

            w.StartObject();
            w.Key("jsonrpc"); w.String("2.0");
            w.Key("method"); w.String("galaxy_data");
            w.Key("params");
                w.StartObject();
                w.Key("eid"); w.Uint(entt::to_integral(_e));
                w.Key("ts"); w.String("insert timestamp here");
                w.Key("name"); w.String(_n.n.c_str());
                w.Key("m"); w.Double(_b.m);
                w.Key("i"); w.Double(_b.i);
                w.Key("r"); w.Double(_r.r);
                w.Key("sc"); w.Uint(int(_s.SpectralClass));
                w.Key("t"); w.Double(_s.Temperature);
                w.Key("spx"); w.Double(_p.v(0)); w.Key("spy"); w.Double(_p.v(1));
                w.EndObject();
            w.EndObject();
            OutputQueue_->enqueue({_ID, s.GetString()});
        });
}

void SimulationManager::queueServerStats(entt::entity _ID)
{
    using namespace rapidjson;

    StringBuffer s;
    Writer<StringBuffer> w(s);

    w.StartObject();
    w.Key("jsonrpc"); w.String("2.0");
    w.Key("method"); w.String("sim_stats");
    w.Key("params");
        w.StartObject();
        w.Key("ts"); w.String("insert timestamp here");
        w.Key("t_sim"); w.Double(SimulationTime_);
        w.Key("t_phy"); w.Double(PhysicsTimer_.elapsed());
        w.Key("t_queue_in"); w.Double(QueueInTimer_.elapsed());
        w.Key("t_queue_out"); w.Double(QueueOutTime_);
        w.Key("stat_sim"); w.Bool(IsSimRunning_);
        w.EndObject();
    w.EndObject();

    OutputQueue_->enqueue({_ID, s.GetString()});
}

void SimulationManager::run()
{
    using namespace rapidjson;

    auto& Messages = Reg_.ctx<MessageHandler>();
    Messages.report("sim", "Simulation Manager running", MessageHandler::INFO);

    IsRunning_ = true;
    while (IsRunning_)
    {
        SimulationTimer_.start();

        NetworkMessage Message;

        QueueInTimer_.start();
        while (QueueSimIn_->try_dequeue(Message))
        {
            Document d;
            d.Parse(Message.Payload.c_str());
            auto& Command = d["params"]["Message"];

            if (Command == "get_data") this->queueGalaxyData(Message.ID);
            else if (Command == "shutdown")  this->shutdown();
            else if (Command == "start_simulation") this->start();
            else if (Command == "stop_simulation")  this->stop();
            else if (Command == "sub_dynamic_data")
            {
                Reg_.emplace_or_replace<DynamicDataSubscriptionComponent>(Message.ID);
            }
        }
        QueueInTimer_.stop();

        PhysicsTimer_.start();
        if (IsSimRunning_)
        {
            // World_->Step(1.0f/60.0f, 8, 3);
            // World2_->Step(1.0f/60.0f, 8, 3);
            // World_->ShiftOrigin({20.0f, 10.0f});
            SysGravity_.calculateForces();
            SysIntegrator_.integrate(3600.0);
        }
        PhysicsTimer_.stop();

        QueueOutTimer_.start();

        Reg_.view<ServerStatusSubscriptionComponent>().each(
            [this](auto _e)
            {
                this->queueServerStats(_e);
            }
        );
        Reg_.view<DynamicDataSubscriptionComponent>().each(
            [this](auto _e)
            {
                this->queueDynamicData(_e);
            }
        );

        QueueOutTimer_.stop();
        QueueOutTime_ = QueueOutTimer_.elapsed();

        SimulationTimer_.stop();
        SimulationTime_ = SimulationTimer_.elapsed();
        if (SimStepSize_ - SimulationTimer_.elapsed_ms() > 0.0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(SimStepSize_ - int(SimulationTimer_.elapsed_ms())));
        }
        else
        {
            Messages.report("sim", "Thread processing exceeds step time ("
                            + std::to_string(SimulationTimer_.elapsed_ms())+"/"
                            + std::to_string(SimStepSize_)+")ms",
                            MessageHandler::WARNING);
        }
    }

    Messages.report("sim", "Simulation thread stopped successfully", MessageHandler::INFO);
}

void SimulationManager::shutdown()
{
    if (IsRunning_)
    {
        auto& Messages = Reg_.ctx<MessageHandler>();

        IsSimRunning_ = false;
        IsRunning_ = false;
        Messages.report("sim","Simulation shutdown", MessageHandler::INFO);
    }
}

void SimulationManager::start()
{
    if (!IsSimRunning_)
    {
        auto& Messages = Reg_.ctx<MessageHandler>();

        IsSimRunning_ = true;
        Messages.report("sim","Simulation started", MessageHandler::INFO);
    }
}

void SimulationManager::stop()
{
    if (IsSimRunning_)
    {
        auto& Messages = Reg_.ctx<MessageHandler>();

        IsSimRunning_ = false;
        Messages.report("sim","Simulation stopped", MessageHandler::INFO);
    }
}
