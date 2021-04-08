#ifndef BODY_COMPONENT_HPP
#define BODY_COMPONENT_HPP

#include "star_definitions.hpp"

struct BodyComponent
{
    double m{1.0}; // mass
    double i{1.0}; // inertia
};

struct StarDataComponent
{
    SpectralClassE SpectralClass{SpectralClassE::M};
    double Temperature{3000.0};
};

struct GravitatorComponent{};

#endif // BODY_COMPONENT_HPP
