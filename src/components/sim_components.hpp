// Sooner or later all simulation related components should be defined here
// for simplicity

#ifndef SIM_COMPONENTS_HPP
#define SIM_COMPONENTS_HPP

#include <vector>
#include <entt/entity/registry.hpp>

struct StarSystemComponent
{
    std::vector<entt::entity> Objects;
    int Seed{0};
};

#endif // SIM_COMPONENTS_HPP
