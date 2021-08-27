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

struct ServerStatusSubscriptionComponent{};

// Subscription to performance from
// 0.1s to 10s update frequency
struct PerformanceStatsSubscriptionTag01{};
struct PerformanceStatsSubscriptionTag05{};
struct PerformanceStatsSubscriptionTag1{};
struct PerformanceStatsSubscriptionTag5{};
struct PerformanceStatsSubscriptionTag10{};

// Subscription to simulation status from
// 0.1s to 10s update frequency
struct SimStatsSubscriptionTag01{};
struct SimStatsSubscriptionTag05{};
struct SimStatsSubscriptionTag1{};
struct SimStatsSubscriptionTag5{};
struct SimStatsSubscriptionTag10{};


struct DynamicDataSubscriptionComponent{};
struct GalaxyDataSubscriptionComponent
{
    bool Transmitted{false};
};

#endif // SUBSCRIPTION_COMPONENTS_HPP
