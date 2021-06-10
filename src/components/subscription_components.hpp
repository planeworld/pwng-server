#ifndef SUBSCRIPTION_COMPONENTS_HPP
#define SUBSCRIPTION_COMPONENTS_HPP

#include <array>

#include <entt/entity/entity.hpp>

constexpr int SYS_MAX = 16;
struct StarSystemsSubscriptionComponent
{
    int Nr{0};
    std::array<entt::entity, SYS_MAX> Systems;
};

// Tags, i.e. components without members

struct ServerStatusSubscriptionComponent
{
};

struct DynamicDataSubscriptionComponent
{
};

#endif // SUBSCRIPTION_COMPONENTS_HPP
