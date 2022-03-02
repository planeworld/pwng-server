#include "galaxy_manager.hpp"

#include "message_handler.hpp"

#include "body_component.hpp"
#include "position_component.hpp"
#include "radius_component.hpp"
#include "star_definitions.hpp"
#include "sim_components.hpp"

void GalaxyManager::generateGalaxy()
{
    auto& Messages = Reg_.ctx<MessageHandler>();

    // std::random_device r;
        // std::default_random_engine Generator(r());
    std::mt19937 Generator;

    std::uniform_int_distribution Seeds;
    std::normal_distribution<double> DistGalaxyRadiusScatter(0.0, 2.0);

    double DensityWaveLength = 1.0e20;
    double GalaxyInnerRadius = 5.0e21;
    double GalaxyOuterRadius = 1.0e22;

    const auto GalaxyDensityMatrixSize = 20;
    std::array<int, GalaxyDensityMatrixSize*GalaxyDensityMatrixSize> GalaxyDensityMatrix;
    for (auto i=0u; i<GalaxyDensityMatrixSize*GalaxyDensityMatrixSize; ++i) GalaxyDensityMatrix[i] = 0;

    struct ProceduralStarTag{};

    double x_max{0.0};
    double y_max{0.0};
    double x_min{0.0};
    double y_min{0.0};
    int c{0};

    // Create inner part of the galaxy
    for (auto r = DensityWaveLength; r < GalaxyInnerRadius; r += DensityWaveLength*(r/GalaxyInnerRadius))
    {
        for (auto t = 0.0; t < 2.0*MATH_PI; t+=0.01 + 0.1*r/GalaxyInnerRadius)
        {
            const auto a = 0.75;
            const auto b = 1.0;

            auto t0 = -r * 1.0e-22 * 8.0;

            // Create elliptic orbit
            #ifdef GALAXY_GENERATION_NO_SCATTERING
                auto x0 = r * a*std::cos(t) + DensityWaveLength;
                auto y0 = r * b*std::sin(t) + DensityWaveLength;
            #else
                auto x0 = r * a*std::cos(t) + DistGalaxyRadiusScatter(Generator) * DensityWaveLength;
                auto y0 = r * b*std::sin(t) + DistGalaxyRadiusScatter(Generator) * DensityWaveLength;
            #endif

            // Rotate elliptic orbit
            auto x = x0 * std::cos(t0) - y0 * std::sin(t0);
            auto y = x0 * std::sin(t0) + y0 * std::cos(t0);

            x_max = std::max(x_max, x);
            y_max = std::max(y_max, y);
            x_min = std::min(x_min, x);
            y_min = std::min(y_min, y);

            auto e = Reg_.create();

            auto e_s = Reg_.create();
            auto SystemComponent = Reg_.emplace<StarSystemComponent>(e_s);
            SystemComponent.Objects = {e};
            SystemComponent.Seed = Seeds(Generator);
            SysName_.setName(e_s, "System_"+std::to_string(c));

            Reg_.emplace<SystemPositionComponent>(e, Vec2Dd{x,y});
            Reg_.emplace<ProceduralStarTag>(e);
            SysName_.setName(e, "Star_"+std::to_string(c));

            ++c;
        }
    }
    // Create outer part of the galaxy
    for (auto r = GalaxyInnerRadius; r<GalaxyOuterRadius; r += DensityWaveLength*(r/GalaxyOuterRadius))
    {
        for (auto t = 0.0; t < 2.0*MATH_PI; t+=0.01 + 0.1*r/GalaxyOuterRadius)
        {
            const auto a = 0.75;
            const auto b = 1.0;

            auto t0 = -r * 2.0e-22 * 4.0;

            // Create elliptic orbit
            #ifdef GALAXY_GENERATION_NO_SCATTERING
                auto x0 = r * a*std::cos(t) + DensityWaveLength;
                auto y0 = r * b*std::sin(t) + DensityWaveLength;
            #else
                auto x0 = r * a*std::cos(t) + DistGalaxyRadiusScatter(Generator) * DensityWaveLength;
                auto y0 = r * b*std::sin(t) + DistGalaxyRadiusScatter(Generator) * DensityWaveLength;
            #endif

            // Rotate elliptic orbit
            auto x = x0 * std::cos(t0) - y0 * std::sin(t0);
            auto y = x0 * std::sin(t0) + y0 * std::cos(t0);

            x_max = std::max(x_max, x);
            y_max = std::max(y_max, y);
            x_min = std::min(x_min, x);
            y_min = std::min(y_min, y);

            auto e = Reg_.create();

            auto e_s = Reg_.create();
            auto SystemComponent = Reg_.emplace<StarSystemComponent>(e_s);
            SystemComponent.Objects = {e};
            SystemComponent.Seed = Seeds(Generator);
            SysName_.setName(e_s, "System_"+std::to_string(c));

            Reg_.emplace<SystemPositionComponent>(e, Vec2Dd{x,y});
            Reg_.emplace<ProceduralStarTag>(e);
            SysName_.setName(e, "Star_"+std::to_string(c));

            ++c;
        }
    }

    Reg_.view<SystemPositionComponent>().each(
        [&](auto _e, const auto& _p)
            {
                auto x_i = int((_p.v[0]-x_min) / (x_max - x_min)*GalaxyDensityMatrixSize);
                auto y_i = int((_p.v[1]-y_min) / (y_max - y_min)*GalaxyDensityMatrixSize);
                GalaxyDensityMatrix[y_i * GalaxyDensityMatrixSize + x_i]++;
            });

    for (auto i=0u; i<GalaxyDensityMatrixSize; ++i)
    {
        for (auto j=0u; j<GalaxyDensityMatrixSize; ++j)
        {
            std::cout << std::setw(5) << GalaxyDensityMatrix[i*GalaxyDensityMatrixSize+j] << " ";
        }
        std::cout << std::endl;
    }

    auto GalaxyDensityMatrixMax = 0;
    for (auto i=0u; i<GalaxyDensityMatrixSize*GalaxyDensityMatrixSize; ++i)
    {
        GalaxyDensityMatrixMax = std::max(GalaxyDensityMatrixMax, GalaxyDensityMatrix[i]);
    }

    std::normal_distribution<double> DistSpectralClass(1.0, 0.25);
    std::normal_distribution<double> DistSpectralClassLowDensity(1.0, 1.0);
    auto View = Reg_.view<SystemPositionComponent, ProceduralStarTag>();

    std::array<double, 7> GalaxySpectralClassFractions{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    for (const auto _e : View)
    {
        const auto& _p = View.get<SystemPositionComponent>(_e);
        auto x_i = int((_p.v[0]-x_min) / (x_max - x_min)*GalaxyDensityMatrixSize);
        auto y_i = int((_p.v[1]-y_min) / (y_max - y_min)*GalaxyDensityMatrixSize);

        auto d = std::sqrt(_p.v(0)*_p.v(0) + _p.v(1)*_p.v(1)) / GalaxyOuterRadius;

        int SpectralClass = DistSpectralClass(Generator)*
            // double(GalaxyDensityMatrix[y_i*GalaxyDensityMatrixSize+x_i])/
            // GalaxyDensityMatrixMax*20000.0*d;
            d*12.0;
        if (GalaxyDensityMatrix[y_i*GalaxyDensityMatrixSize+x_i] <= 20) SpectralClass = DistSpectralClassLowDensity(Generator)*2.5;
        if (GalaxyDensityMatrix[y_i*GalaxyDensityMatrixSize+x_i] <= 10) SpectralClass = DistSpectralClassLowDensity(Generator);
        if (SpectralClass < 0) SpectralClass = 0;
        if (SpectralClass > 6) SpectralClass = 6;

        Reg_.emplace<StarDataComponent>(_e, SpectralClassE(SpectralClass), StarTemperatureDistribution[SpectralClass](Generator));
        Reg_.emplace<BodyComponent>(_e, StarMassDistribution[SpectralClass](Generator), 1.0);
        Reg_.emplace<RadiusComponent>(_e, StarRadiusDistribution[SpectralClass](Generator));

        DBLK(GalaxySpectralClassFractions[SpectralClass] += 1.0;)
            // DBLK(Messages.reportRaw(std::to_string(SpectralClass)+" ", MessageHandler::DEBUG_L3);)
    };
    Reg_.clear<ProceduralStarTag>();

    DBLK(
        std::cout << "Spectral class distribution: " << std::endl;
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "  M: " << GalaxySpectralClassFractions[0]/c*100.0 << "%" << std::endl;
        std::cout << "  K: " << GalaxySpectralClassFractions[1]/c*100.0 << "%" << std::endl;
        std::cout << "  G: " << GalaxySpectralClassFractions[2]/c*100.0 << "%" << std::endl;
        std::cout << "  F: " << GalaxySpectralClassFractions[3]/c*100.0 << "%" << std::endl;
        std::cout << "  A: " << GalaxySpectralClassFractions[4]/c*100.0 << "%" << std::endl;
        std::cout << "  B: " << GalaxySpectralClassFractions[5]/c*100.0 << "%" << std::endl;
        std::cout << "  O: " << GalaxySpectralClassFractions[6]/c*100.0 << "%" << std::endl;
    )
        // std::uniform_int_distribution Seeds;
        // std::normal_distribution<double> DistGalaxyArmScatter(0.0, 1.0);
        // std::normal_distribution<double> DistGalaxyArmScatterBase(0.1, 0.05);
        // std::normal_distribution<double> DistGalaxyArmScatterDeviation(0.0, 0.02);
        // std::normal_distribution<double> DistGalaxyArmLengthBase(0.15, 0.1);
        // std::normal_distribution<double> DistGalaxyArmLengthDeviation(0.0, 0.05);
        // std::normal_distribution<double> DistGalaxyCenter(0.0, 0.5);
        // std::normal_distribution<double> DistSpectralClass(0.0, 1.0);
        // std::poisson_distribution<int> DistGalaxyArms(4);
        // std::poisson_distribution<int> DistNumberOfPlanets(3.5);

        // int Arms=DistGalaxyArms(Generator);
        // // if (Arms < 2) Arms = 2;
        // Arms = 2;
        // Messages.report("sim", "Creating galaxy with " + std::to_string(Arms) + " spiral arms", MessageHandler::INFO);
        // double Alpha = 1.0e22;
        // // double ScatterBase = 0.1; // 10%
        // // double GalaxyPhiMin = 0.2 * MATH_PI;
        // double GalaxyPhiMin = DistGalaxyArmLengthBase(Generator) * MATH_PI;
        // if (GalaxyPhiMin < 0.1 * MATH_PI) GalaxyPhiMin = 0.2 * MATH_PI;
        // double GalaxyPhiMax = 4.0 * MATH_PI;
        // double GalaxyRadiusMax = Alpha / GalaxyPhiMin;
        // double GalaxyDistanceMean = 5.0e16;

        // double GalaxyScatterBase = DistGalaxyArmScatterBase(Generator);
        // DBLK(Messages.report("sim", "Scatter baseline for galaxy arms " + std::to_string(GalaxyScatterBase), MessageHandler::DEBUG_L2);)

        // DBLK(Messages.report("sim", "Creating spiral arms", MessageHandler::DEBUG_L1);)
        // DBLK(Messages.report("sim", "Distribution of spectral classes (0-6 = M-O):", MessageHandler::DEBUG_L3);)

        // int c{0};
        // for (auto i=0; i<Arms; ++i)
        // {
        //     double ScatterArm = GalaxyScatterBase + DistGalaxyArmScatterDeviation(Generator);
        //     if (ScatterArm < 0.05) ScatterArm = 0.05;
        //     DBLK(Messages.report("sim", "Scatter baseline for galaxy arm " + std::to_string(i) + ": "
        //                          + std::to_string(ScatterArm), MessageHandler::DEBUG_L2);)

        //     for (auto Phi=GalaxyPhiMin+DistGalaxyArmLengthDeviation(Generator)*MATH_PI; Phi<GalaxyPhiMax; Phi+=0.005)
        //     // for (auto Phi=0.15; Phi<GalaxyPhiMax; Phi+=0.005)
        //     {
        //         auto e = Reg_.create();

        //         auto e_s = Reg_.create();
        //         auto SystemComponent = Reg_.emplace<StarSystemComponent>(e_s);
        //         SystemComponent.Objects = {e};
        //         SystemComponent.Seed = Seeds(Generator);
        //         SysName_.setName(e_s, "System_"+std::to_string(c));

        //         double r=Alpha/Phi;
        //         double p = Phi+2.0*MATH_PI/Arms*i;
        //         double r_n = 1.0-Phi/GalaxyPhiMax;
        //         DistSpectralClass = std::normal_distribution<double>(r_n, 0.16);
        //         int SpectralClass = DistSpectralClass(Generator)*6;
        //         if (SpectralClass < 0) SpectralClass = 0;
        //         if (SpectralClass > 6) SpectralClass = 6;
        //         DBLK(Messages.reportRaw(std::to_string(SpectralClass)+" ", MessageHandler::DEBUG_L3);)

        //         Reg_.emplace<SystemPositionComponent>(e, Vec2Dd{r*std::cos(p)+DistGalaxyArmScatter(Generator)*r*ScatterArm,
        //                                                         r*std::sin(p)+DistGalaxyArmScatter(Generator)*r*ScatterArm});
        //         Reg_.emplace<BodyComponent>(e, StarMassDistribution[SpectralClass](Generator), 1.0);
        //         Reg_.emplace<StarDataComponent>(e, SpectralClassE(SpectralClass), StarTemperatureDistribution[SpectralClass](Generator));
        //         Reg_.emplace<RadiusComponent>(e, StarRadiusDistribution[SpectralClass](Generator));
        //         SysName_.setName(e, "Star_"+std::to_string(c));
        //         ++c;
        //     }
        //     DBLK(Messages.reportRaw("\n", MessageHandler::DEBUG_L3);)
        // }
        // DBLK(Messages.report("sim", "Creating center", MessageHandler::DEBUG_L1);)
        // DBLK(Messages.report("sim", "Distribution of spectral classes (0-6 = M-O):", MessageHandler::DEBUG_L3);)
        // for (auto Phi=0.0; Phi<2.0*MATH_PI; Phi+=0.001)
        // {
        //     auto e = Reg_.create();

        //     auto e_s = Reg_.create();
        //     auto SystemComponent = Reg_.emplace<StarSystemComponent>(e_s);
        //     SystemComponent.Objects = {e};
        //     SystemComponent.Seed = Seeds(Generator);
        //     SysName_.setName(e_s, "System_"+std::to_string(c));

        //     double r=std::abs(DistGalaxyCenter(Generator));

        //     DistSpectralClass = std::normal_distribution<double>(r, 0.16);
        //     int SpectralClass = DistSpectralClass(Generator)*6;
        //     if (SpectralClass < 0) SpectralClass = 0;
        //     if (SpectralClass > 6) SpectralClass = 6;
        //     DBLK(Messages.reportRaw(std::to_string(SpectralClass)+" ", MessageHandler::DEBUG_L3);)

        //     Reg_.emplace<SystemPositionComponent>(e, 0.5e22*r*Vec2Dd{std::cos(Phi),std::sin(Phi)});
        //     Reg_.emplace<BodyComponent>(e, StarMassDistribution[SpectralClass](Generator), 1.0);
        //     Reg_.emplace<StarDataComponent>(e, SpectralClassE(SpectralClass), StarTemperatureDistribution[SpectralClass](Generator));
        //     Reg_.emplace<RadiusComponent>(e, StarRadiusDistribution[SpectralClass](Generator));
        //     SysName_.setName(e, "Star_"+std::to_string(c));
        //     ++c;
        // }
        // DBLK(Messages.reportRaw("\n", MessageHandler::DEBUG_L3);)

    Messages.report("sim", std::to_string(c) + " star systems generated", MessageHandler::INFO);
}
