#include "simulation_manager.hpp"

#include <random>

#include <rapidjson/document.h>

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

    if (World_ != nullptr)
    {
        delete World_;
        World_ = nullptr;
    }
}

void SimulationManager::init(moodycamel::ConcurrentQueue<NetworkMessage>* const _QueueSimIn,
                             moodycamel::ConcurrentQueue<NetworkMessage>* const _OutputQueue)
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    Messages.report("sim", "Initialising Simulation Manager", MessageHandler::INFO);

    QueueSimIn_ = _QueueSimIn;
    OutputQueue_ = _OutputQueue;

    World_ = new b2World({0.0f, -9.81f});

    this->createTire();

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
    Reg_.emplace<RadiusComponent>(Earth, 6378137.0);
    SysName_.setName(Earth, "Earth");

    auto Moon = Reg_.create();
    Reg_.emplace<SystemPositionComponent>(Moon, SolarSystemPosition);
    Reg_.emplace<PositionComponent>(Moon, Vec2Dd{384400.0e3, -152.1e9});
    Reg_.emplace<VelocityComponent>(Moon, Vec2Dd{29.29e3, 964.0});
    Reg_.emplace<AccelerationComponent>(Moon, Vec2Dd{0.0, 0.0});
    Reg_.emplace<BodyComponent>(Moon, 7.346e22, 1.0);
    Reg_.emplace<RadiusComponent>(Moon, 1737.0e3);
    SysName_.setName(Moon, "Moon");

    auto Sun = Reg_.create();
    Reg_.emplace<SystemPositionComponent>(Sun, SolarSystemPosition);
    Reg_.emplace<PositionComponent>(Sun, Vec2Dd{0.0, 0.0});
    Reg_.emplace<VelocityComponent>(Sun, Vec2Dd{0.0, 0.0});
    Reg_.emplace<AccelerationComponent>(Sun, Vec2Dd{0.0, 0.0});
    Reg_.emplace<BodyComponent>(Sun, 1.9884e30, 1.0);
    Reg_.emplace<StarDataComponent>(Sun, SpectralClassE::G, 5778.0);
    Reg_.emplace<RadiusComponent>(Sun, 6.96342e8);
    SysName_.setName(Sun, "Sun");

    std::mt19937 Generator;

    std::uniform_int_distribution Seeds;
    auto SolarSystem = Reg_.create();
    auto& SolarSystemComponent = Reg_.emplace<StarSystemComponent>(SolarSystem);
    SolarSystemComponent.Objects = {Sun, Earth, Moon};
    SolarSystemComponent.Seed = Seeds(Generator);
    SysName_.setName(SolarSystem, "Solar System");

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

            auto e_s = Reg_.create();
            auto SystemComponent = Reg_.emplace<StarSystemComponent>(e_s);
            SystemComponent.Objects = {e};
            SystemComponent.Seed = Seeds(Generator);
            SysName_.setName(e_s, "System_"+std::to_string(c));

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
            Reg_.emplace<RadiusComponent>(e, StarRadiusDistribution[SpectralClass](Generator));
            SysName_.setName(e, "Star_"+std::to_string(c));
            ++c;
        }
    }
    DBLK(std::cout << std::endl;)
    DBLK(Messages.report("sim", "Creating center", MessageHandler::DEBUG_L1);)
    DBLK(Messages.report("sim", "Distribution of spectral classes (0-6 = M-O):", MessageHandler::DEBUG_L3);)
    for (auto Phi=0.0; Phi<2.0*MATH_PI; Phi+=0.001)
    {
        auto e = Reg_.create();

        auto e_s = Reg_.create();
        auto SystemComponent = Reg_.emplace<StarSystemComponent>(e_s);
        SystemComponent.Objects = {e};
        SystemComponent.Seed = Seeds(Generator);
        SysName_.setName(e_s, "System_"+std::to_string(c));

        double r=std::abs(DistGalaxyCenter(Generator));

        DistSpectralClass = std::normal_distribution<double>(r, 0.16);
        int SpectralClass = DistSpectralClass(Generator)*6;
        if (SpectralClass < 0) SpectralClass = 0;
        if (SpectralClass > 6) SpectralClass = 6;
        DBLK(std::cout << SpectralClass << " ";)

        Reg_.emplace<SystemPositionComponent>(e, 0.5e22*r*Vec2Dd{std::cos(Phi),std::sin(Phi)});
        Reg_.emplace<BodyComponent>(e, StarMassDistribution[SpectralClass](Generator), 1.0);
        Reg_.emplace<StarDataComponent>(e, SpectralClassE(SpectralClass), StarTemperatureDistribution[SpectralClass](Generator));
        Reg_.emplace<RadiusComponent>(e, StarRadiusDistribution[SpectralClass](Generator));
        SysName_.setName(e, "Star_"+std::to_string(c));
        ++c;
    }
    DBLK(std::cout << std::endl;)
    Messages.report("sim", std::to_string(c) + " star systems generated", MessageHandler::INFO);

    Thread_ = std::thread(&SimulationManager::run, this);
    Messages.report("sim", "Simulation thread started successfully", MessageHandler::INFO);

}

std::uint64_t SimulationManager::getTimeStamp() const
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void SimulationManager::queueDynamicData(entt::entity _ClientID) const
{
    auto& Json = Reg_.ctx<JsonManager>();

    Reg_.view<BodyComponent,
              NameComponent,
              PositionComponent,
              RadiusComponent,
              SystemPositionComponent>().each
        ([&](auto _e, const auto& _b, const auto& _n, const auto& _p,
                      const auto& _r, const auto& _s)
        {
            Json.createNotification("bc_dynamic_data")
                .addParam("eid", entt::to_integral(_e))
                .addParam("ts", SimTime_.toStamp())
                .addParam("ts_r", this->getTimeStamp())
                .addParam("name", _n.Name)
                .addParam("m", _b.m)
                .addParam("i", _b.i)
                .addParam("r", _r.r)
                .addParam("spx", _s.v(0))
                .addParam("spy", _s.v(1))
                .addParam("px", _p.v(0))
                .addParam("py", _p.v(1))
                .finalise();
            OutputQueue_->enqueue({_ClientID, Json.getString()});
        });
}

void SimulationManager::queueGalaxyData(entt::entity _ClientID, JsonManager::RequestIDType _ReqID) const
{
    auto& Json = Reg_.ctx<JsonManager>();

    // Queue stars of the galaxy
    Reg_.view<SystemPositionComponent,
              BodyComponent,
              RadiusComponent,
              StarDataComponent,
              NameComponent>().each
        ([&](auto _e, const auto& _p, const auto& _b, const auto& _r, const auto& _s, const auto& _n)
        {
            Json.createNotification("galaxy_data_stars")
                .addParam("eid", entt::to_integral(_e))
                .addParam("ts", SimTime_.toStamp())
                .addParam("ts_r", this->getTimeStamp())
                .addParam("name", _n.Name)
                .addParam("m", _b.m)
                .addParam("i", _b.i)
                .addParam("r", _r.r)
                .addParam("sc", std::uint32_t(_s.SpectralClass))
                .addParam("t", _s.Temperature)
                .addParam("spx", _p.v(0))
                .addParam("spy", _p.v(1))
                .finalise();
            OutputQueue_->enqueue({_ClientID, Json.getString()});
        });

    // Queue star systems
    Reg_.view<NameComponent, StarSystemComponent>().each
        ([&](auto _e, const auto& _n, const auto& _s)
        {
            Json.createNotification("galaxy_data_systems")
                .addParam("eid", entt::to_integral(_e))
                .addParam("ts", SimTime_.toStamp())
                .addParam("ts_r", this->getTimeStamp())
                .addParam("name", _n.Name)
                .finalise();
            OutputQueue_->enqueue({_ClientID, Json.getString()});
        });

    Json.createResult("success")
        .finalise(_ReqID);
    OutputQueue_->enqueue({_ClientID, Json.getString()});
}

void SimulationManager::queueServerStats(entt::entity _ClientID)
{
    auto& Json = Reg_.ctx<JsonManager>();

    Json.createNotification("sim_stats")
        .addParam("ts", SimTime_.toStamp())
        .addParam("ts_r", this->getTimeStamp())
        .addParam("t_sim", SimulationTime_)
        .addParam("t_phy", PhysicsTimer_.elapsed())
        .addParam("t_queue_in", QueueInTimer_.elapsed())
        .addParam("t_queue_out", QueueOutTime_)
        .addParam("stat_sim", IsSimRunning_)
        .finalise();

    OutputQueue_->enqueue({_ClientID, Json.getString()});
}

void SimulationManager::queueTireData(entt::entity _ClientID) const
{
    auto& Json = Reg_.ctx<JsonManager>();

    Reg_.view<TireComponent>().each
        ([&](auto _e, const auto& _t)
        {
            Json.createNotification("tire_data")
                .addParam("eid", entt::to_integral(_e))
                .addParam("ts", SimTime_.toStamp())
                .addParam("ts_r", this->getTimeStamp())
                .beginArray("rim_xy")
                .addValue(_t.Rim->GetWorldCenter().x)
                .addValue(_t.Rim->GetWorldCenter().y)
                .endArray()
                .addParam("rim_r", _t.Rim->GetFixtureList()->GetShape()->m_radius)
                .beginArray("rubber");

            for (auto r : _t.Rubber)
            {
                Json.addValue(r->GetWorldCenter().x)
                    .addValue(r->GetWorldCenter().y);
            }

            Json.endArray()
                .finalise();

            OutputQueue_->enqueue({_ClientID, Json.getString()});
        });

}

void SimulationManager::run()
{
    using namespace rapidjson;

    auto& Messages = Reg_.ctx<MessageHandler>();
    Messages.report("sim", "Simulation Manager running", MessageHandler::INFO);

    Timer TimerServerStatusSubscription;
    TimerServerStatusSubscription.start();

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
            auto& Command = d["method"];

            if (Command == "get_data") this->queueGalaxyData(Message.ClientID, d["id"].GetUint());
            else if (Command == "shutdown")  this->shutdown();
            else if (Command == "start_simulation") this->start();
            else if (Command == "stop_simulation")  this->stop();
            else if (Command == "sub_dynamic_data")
            {
                Reg_.emplace_or_replace<DynamicDataSubscriptionComponent>(Message.ClientID);
            }
            else if (Command == "sub_system")
            {
                auto& Json = Reg_.ctx<JsonManager>();
                Json.createResult(true)
                    .finalise(d["id"].GetUint());
                OutputQueue_->enqueue({Message.ClientID, Json.getString()});
            }
        }
        QueueInTimer_.stop();

        PhysicsTimer_.start();
        if (IsSimRunning_)
        {
            World_->Step(SimStepSize_*1.0e-3, 8, 3);
            SysGravity_.calculateForces();
            SysIntegrator_.integrate(3600.0);
            SimTime_.inc(SimStepSize_*1.0e-3*3600.0);
        }
        PhysicsTimer_.stop();

        QueueOutTimer_.start();

        if (TimerServerStatusSubscription.time() >= 0.1)
        {
            Reg_.view<ServerStatusSubscriptionComponent>().each(
                [this](auto _e)
                {
                    this->queueServerStats(_e);
                });
            TimerServerStatusSubscription.restart();

        }
        Reg_.view<DynamicDataSubscriptionComponent>().each(
            [this](auto _e)
            {
                this->queueDynamicData(_e);
                this->queueTireData(_e);
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
        SimTime_.start();
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

void SimulationManager::createTire()
{
    b2BodyDef myBodyDef;
    myBodyDef.type = b2_staticBody; //change body type
    myBodyDef.position.Set(0,-0.5); //middle, bottom

    b2PolygonShape polygonShape;
    polygonShape.SetAsBox(150,0.5); //ends of the line

    b2FixtureDef myFixtureDef;
    myFixtureDef.density = 1.0f;
    // myFixtureDef.restitution = 1.0f;
    myFixtureDef.shape = &polygonShape;
    b2Body* staticBody = World_->CreateBody(&myBodyDef);
    staticBody->CreateFixture(&myFixtureDef); //add a fixture to the body

    float p_x{0.0f};
    float p_y{0.5f};

    auto e = Reg_.create();
    auto& Tire = Reg_.emplace<TireComponent>(e);

    b2BodyDef BodyDefRim;
    BodyDefRim.type = b2_dynamicBody;
    BodyDefRim.position.Set(p_x, p_y);

    b2CircleShape ShapeCircleRim;
    ShapeCircleRim.m_p.Set(0.0f, 0.0f);
    ShapeCircleRim.m_radius = 0.2;
    b2FixtureDef FixtureDefRim;
    FixtureDefRim.density = 80.0;
    FixtureDefRim.shape = &ShapeCircleRim;

    Tire.Rim = World_->CreateBody(&BodyDefRim);
    Tire.Rim->CreateFixture(&FixtureDefRim);

    for (auto i=0u; i<TireComponent::SEGMENTS; ++i)
    {
        double Angle = 3.14159 * 2.0 / TireComponent::SEGMENTS * i;

        // Position on rim
        float x_r = p_x;
        float y_r = p_y;
        // float x_r = std::sin(Angle)*0.2 + p_x;
        // float y_r = std::cos(Angle)*0.2 + p_y;
        // Position on tire
        float x_t = std::sin(Angle)*0.4 + p_x;
        float y_t = std::cos(Angle)*0.4 + p_y;

        // Define the body
        b2BodyDef BodyDefTire;
        BodyDefTire.bullet = false;
        BodyDefTire.fixedRotation = true;
        BodyDefTire.type = b2_dynamicBody;
        BodyDefTire.position.Set(x_t, y_t);

        // Define the shape and its fixture
        b2CircleShape ShapeCircleTire;
        ShapeCircleTire.m_p.Set(0, 0);
        ShapeCircleTire.m_radius = 0.01;
        b2FixtureDef FixtureDefTire;
        FixtureDefTire.density = 1.0;
        FixtureDefTire.shape = &ShapeCircleTire;

        // Create body and attach shape
        Tire.Rubber[i] = World_->CreateBody(&BodyDefTire);
        Tire.Rubber[i]->CreateFixture(&FixtureDefTire);

        b2MassData MassData;
        Tire.Rubber[i]->GetMassData(&MassData);
        MassData.mass = 10.0f / TireComponent::SEGMENTS;
        Tire.Rubber[i]->SetMassData(&MassData);

        // Define joint
        b2DistanceJointDef JointDefRadial;
        JointDefRadial.Initialize(Tire.Rim, Tire.Rubber[i],
                                  {x_r, y_r}, {x_t, y_t});
        JointDefRadial.collideConnected = true;
        b2LinearStiffness(JointDefRadial.stiffness, JointDefRadial.damping, 0.1f, 0.1f, Tire.Rim, Tire.Rubber[i]);

        Tire.RadialJoints[i] = static_cast<b2DistanceJoint*>(World_->CreateJoint(&JointDefRadial));
        Tire.RadialJoints[i]->SetMinLength(0.35f);
        // Tire.RadialJoints[i]->SetMaxLength(0.45f);
        // Tire.RadialJoints[i]->SetLength(0.4f);

        if (i>0u)
        {
            b2DistanceJointDef JointDefTangential;
            JointDefTangential.Initialize(Tire.Rubber[i], Tire.Rubber[i-1],
                                          {x_t, y_t}, Tire.Rubber[i-1]->GetWorldCenter());
            JointDefTangential.collideConnected = true;
            b2LinearStiffness(JointDefTangential.stiffness, JointDefTangential.damping, 0.1f, 0.1f, Tire.Rubber[i], Tire.Rubber[i-1]);

            Tire.TangentialJoints[i] = static_cast<b2DistanceJoint*>(World_->CreateJoint(&JointDefTangential));
            // Tire.TangentialJoints[i]->SetMinLength(0.01f);
        }
    }
    b2DistanceJointDef JointDefTangential;
    JointDefTangential.Initialize(Tire.Rubber[TireComponent::SEGMENTS-1], Tire.Rubber[0],
                                  Tire.Rubber[TireComponent::SEGMENTS-1]->GetWorldCenter(), Tire.Rubber[0]->GetWorldCenter());
    JointDefTangential.collideConnected = true;
    b2LinearStiffness(JointDefTangential.stiffness, JointDefTangential.damping, 0.1f, 0.1f, Tire.Rubber[0], Tire.Rubber[TireComponent::SEGMENTS]);

    Tire.TangentialJoints[TireComponent::SEGMENTS-1] = static_cast<b2DistanceJoint*>(World_->CreateJoint(&JointDefTangential));
    // Tire.TangentialJoints[TireComponent::SEGMENTS-1]->SetMinLength(0.01f);

    b2MassData MassData;
    Tire.Rim->GetMassData(&MassData);
    MassData.mass = 10.0f;
    Tire.Rim->SetMassData(&MassData);

    std::cout << "Mass (Rim): " << Tire.Rim->GetMass() << "kg" << std::endl;
    std::cout << "Mass (Rubber): " << Tire.Rubber[0]->GetMass()*TireComponent::SEGMENTS << "kg" << std::endl;
}
