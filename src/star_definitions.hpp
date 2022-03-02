#ifndef STAR_DEFINITIONS_HPP
#define STAR_DEFINITIONS_HPP

#include <array>
#include <random>

// Based on Harvard spectral classification
// Looking at main sequence stars only for now

enum class SpectralClassE : int
{
    M = 0,
    K = 1,
    G = 2,
    F = 3,
    A = 4,
    B = 5,
    O = 6
};

constexpr double SOLAR_LUMINOSITY{3.828e26}; // W
constexpr double SOLAR_MASSES{6.96342e8}; // kg
constexpr double SOLAR_RADIUS{6.957e8}; // m

// Parameters for random distributions
// Normal distribution, values are mean and standard deviation
// Array index represents spectral class

static std::array<std::pair<double, double>, 7> StarMassDistributionParams{{
    { 0.3*SOLAR_MASSES, 0.125*SOLAR_MASSES},
    { 0.8*SOLAR_MASSES, 0.075*SOLAR_MASSES},
    { 1.1*SOLAR_MASSES,   0.1*SOLAR_MASSES},
    { 1.7*SOLAR_MASSES,   0.3*SOLAR_MASSES},
    { 3.2*SOLAR_MASSES,  0.75*SOLAR_MASSES},
    {18.0*SOLAR_MASSES,   7.0*SOLAR_MASSES},
    {60.0*SOLAR_MASSES,  15.0*SOLAR_MASSES}
}};
static std::array<std::pair<double, double>, 7> StarRadiusDistributionParams{{
    {  0.6*SOLAR_RADIUS,   0.05*SOLAR_RADIUS},
    { 0.83*SOLAR_RADIUS,  0.065*SOLAR_RADIUS},
    {1.055*SOLAR_RADIUS, 0.0475*SOLAR_RADIUS},
    {1.275*SOLAR_RADIUS, 0.0625*SOLAR_RADIUS},
    {  1.6*SOLAR_RADIUS,    0.1*SOLAR_RADIUS},
    {  4.2*SOLAR_RADIUS,    1.2*SOLAR_RADIUS},
    { 12.5*SOLAR_RADIUS,   2.95*SOLAR_RADIUS} // Assumption: largest main sequence star is 18.4 times solar radius
}};
static std::array<std::pair<double, double>, 7> StarLuminosityDistributionParams{{
    { 1.0e-2*SOLAR_LUMINOSITY,   0.05*SOLAR_LUMINOSITY},
    { 0.83*SOLAR_LUMINOSITY,  0.065*SOLAR_LUMINOSITY},
    {1.055*SOLAR_LUMINOSITY, 0.0475*SOLAR_LUMINOSITY},
    {1.275*SOLAR_LUMINOSITY, 0.0625*SOLAR_LUMINOSITY},
    {  1.6*SOLAR_LUMINOSITY,    0.1*SOLAR_LUMINOSITY},
    {  4.2*SOLAR_LUMINOSITY,    1.2*SOLAR_LUMINOSITY},
    { 1.0e6*SOLAR_LUMINOSITY,   2.95*SOLAR_LUMINOSITY} // Assumption: largest main sequence star is 18.4 times solar radius
}};
static std::array<std::pair<double, double>, 7> StarTemperatureDistributionParams{{
    {3050.0, 325.0},
    // {2050.0, 1325.0},
    {4450.0, 375.0},
    {5600.0, 200.0},
    {6750.0, 375.0},
    {8759.0, 625.0},
    {20000.0, 5000.0},
    {40000.0, 5000.0}
}};

static std::array<std::normal_distribution<double>, 7> StarMassDistribution
{
   std::normal_distribution<double>{StarMassDistributionParams[int(SpectralClassE::M)].first,
                                    StarMassDistributionParams[int(SpectralClassE::M)].second},
   std::normal_distribution<double>{StarMassDistributionParams[int(SpectralClassE::K)].first,
                                    StarMassDistributionParams[int(SpectralClassE::K)].second},
   std::normal_distribution<double>{StarMassDistributionParams[int(SpectralClassE::G)].first,
                                    StarMassDistributionParams[int(SpectralClassE::G)].second},
   std::normal_distribution<double>{StarMassDistributionParams[int(SpectralClassE::F)].first,
                                    StarMassDistributionParams[int(SpectralClassE::F)].second},
   std::normal_distribution<double>{StarMassDistributionParams[int(SpectralClassE::A)].first,
                                    StarMassDistributionParams[int(SpectralClassE::A)].second},
   std::normal_distribution<double>{StarMassDistributionParams[int(SpectralClassE::B)].first,
                                    StarMassDistributionParams[int(SpectralClassE::B)].second},
   std::normal_distribution<double>{StarMassDistributionParams[int(SpectralClassE::O)].first,
                                    StarMassDistributionParams[int(SpectralClassE::O)].second}
};

static std::array<std::normal_distribution<double>, 7> StarRadiusDistribution
{
   std::normal_distribution<double>{StarRadiusDistributionParams[int(SpectralClassE::M)].first,
                                    StarRadiusDistributionParams[int(SpectralClassE::M)].second},
   std::normal_distribution<double>{StarRadiusDistributionParams[int(SpectralClassE::K)].first,
                                    StarRadiusDistributionParams[int(SpectralClassE::K)].second},
   std::normal_distribution<double>{StarRadiusDistributionParams[int(SpectralClassE::G)].first,
                                    StarRadiusDistributionParams[int(SpectralClassE::G)].second},
   std::normal_distribution<double>{StarRadiusDistributionParams[int(SpectralClassE::F)].first,
                                    StarRadiusDistributionParams[int(SpectralClassE::F)].second},
   std::normal_distribution<double>{StarRadiusDistributionParams[int(SpectralClassE::A)].first,
                                    StarRadiusDistributionParams[int(SpectralClassE::A)].second},
   std::normal_distribution<double>{StarRadiusDistributionParams[int(SpectralClassE::B)].first,
                                    StarRadiusDistributionParams[int(SpectralClassE::B)].second},
   std::normal_distribution<double>{StarRadiusDistributionParams[int(SpectralClassE::O)].first,
                                    StarRadiusDistributionParams[int(SpectralClassE::O)].second}
};

static std::array<std::normal_distribution<double>, 7> StarTemperatureDistribution
{
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClassE::M)].first,
                                    StarTemperatureDistributionParams[int(SpectralClassE::M)].second},
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClassE::K)].first,
                                    StarTemperatureDistributionParams[int(SpectralClassE::K)].second},
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClassE::G)].first,
                                    StarTemperatureDistributionParams[int(SpectralClassE::G)].second},
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClassE::F)].first,
                                    StarTemperatureDistributionParams[int(SpectralClassE::F)].second},
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClassE::A)].first,
                                    StarTemperatureDistributionParams[int(SpectralClassE::A)].second},
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClassE::B)].first,
                                    StarTemperatureDistributionParams[int(SpectralClassE::B)].second},
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClassE::O)].first,
                                    StarTemperatureDistributionParams[int(SpectralClassE::O)].second}
};

#endif // STAR_DEFINITIONS_HPP
