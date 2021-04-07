#ifndef STAR_DEFINITIONS_HPP
#define STAR_DEFINITIONS_HPP

#include <array>
#include <random>

// Based on Harvard spectral classification
// Looking at main sequence stars only for now

enum class SpectralClass : int
{
    M = 0,
    K = 1,
    G = 2,
    F = 3,
    A = 4,
    B = 5,
    O = 6
};

constexpr double SOLAR_MASSES{6.96342e8};
constexpr double SOLAR_RADIUS{6.957e8};

// Parameters for random distributions
// Normal distribution, values are mean and standard deviation
// Array index represents spectral class

std::array<std::pair<double, double>, 7> StarMassDistributionParams{{
    { 0.3*SOLAR_MASSES, 0.125*SOLAR_MASSES},
    { 0.8*SOLAR_MASSES, 0.075*SOLAR_MASSES},
    { 1.1*SOLAR_MASSES,   0.1*SOLAR_MASSES},
    { 1.7*SOLAR_MASSES,   0.3*SOLAR_MASSES},
    { 3.2*SOLAR_MASSES,  0.75*SOLAR_MASSES},
    {18.0*SOLAR_MASSES,   7.0*SOLAR_MASSES},
    {60.0*SOLAR_MASSES,  15.0*SOLAR_MASSES}
}};
std::array<std::pair<double, double>, 7> StarRadiusDistributionParams{{
    {  0.6*SOLAR_RADIUS,   0.05*SOLAR_RADIUS},
    { 0.83*SOLAR_RADIUS,  0.065*SOLAR_RADIUS},
    {1.055*SOLAR_RADIUS, 0.0475*SOLAR_RADIUS},
    {1.275*SOLAR_RADIUS, 0.0625*SOLAR_RADIUS},
    {  1.6*SOLAR_RADIUS,    0.1*SOLAR_RADIUS},
    {  4.2*SOLAR_RADIUS,    1.2*SOLAR_RADIUS},
    { 12.5*SOLAR_RADIUS,   2.95*SOLAR_RADIUS} // Assumption: largest main sequence star is 18.4 times solar radius
}};
std::array<std::pair<double, double>, 7> StarTemperatureDistributionParams{{
    {3050.0, 325.0},
    {4450.0, 375.0},
    {5600.0, 200.0},
    {6750.0, 375.0},
    {8759.0, 625.0},
    {20000.0, 5000.0},
    {40000.0, 5000.0}
}};


std::array<std::normal_distribution<double>, 7> StarTemperatureDistribution
{
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClass::M)].first,
                                    StarTemperatureDistributionParams[int(SpectralClass::M)].second},
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClass::K)].first,
                                    StarTemperatureDistributionParams[int(SpectralClass::K)].second},
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClass::G)].first,
                                    StarTemperatureDistributionParams[int(SpectralClass::G)].second},
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClass::F)].first,
                                    StarTemperatureDistributionParams[int(SpectralClass::F)].second},
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClass::A)].first,
                                    StarTemperatureDistributionParams[int(SpectralClass::A)].second},
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClass::B)].first,
                                    StarTemperatureDistributionParams[int(SpectralClass::B)].second},
   std::normal_distribution<double>{StarTemperatureDistributionParams[int(SpectralClass::O)].first,
                                    StarTemperatureDistributionParams[int(SpectralClass::O)].second}
};
#endif // STAR_DEFINITIONS_HPP
