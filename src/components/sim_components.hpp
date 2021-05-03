// Sooner or later all simulation related components should be defined here
// for simplicity

#ifndef SIM_COMPONENTS_HPP
#define SIM_COMPONENTS_HPP

#include <vector>
#include <entt/entity/registry.hpp>

struct StarSystemComponent
{
    std::vector<entt::entity> Objects;
};

// Definition of tags, i.e. components without any members
// Could use entt::tag<"constexprstr"_hs> here, but I see
// no benefit at the moment and it's more clear/readable
// for me.


#endif // SIM_COMPONENTS_HPP
