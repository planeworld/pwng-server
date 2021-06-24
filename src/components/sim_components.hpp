// Sooner or later all simulation related components should be defined here
// for simplicity

#ifndef SIM_COMPONENTS_HPP
#define SIM_COMPONENTS_HPP

#include <array>
#include <vector>
#include <box2d/box2d.h>
#include <entt/entity/registry.hpp>

struct TireComponent
{
    constexpr static int SEGMENTS = 64;

    std::array<b2Body*, SEGMENTS> Rubber;
    b2Body* Rim;

    std::array<b2DistanceJoint*, SEGMENTS> RadialJoints;
    std::array<b2DistanceJoint*, SEGMENTS> TangentialJoints;
};

struct StarSystemComponent
{
    std::vector<entt::entity> Objects;
    int Seed{0};
};

#endif // SIM_COMPONENTS_HPP
