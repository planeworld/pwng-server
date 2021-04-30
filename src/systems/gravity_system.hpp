#ifndef GRAVITY_SYSTEM_HPP
#define GRAVITY_SYSTEM_HPP

#include <iostream>

#include <entt/entity/registry.hpp>

#include "acceleration_component.hpp"
#include "body_component.hpp"
#include "math_types.hpp"
#include "position_component.hpp"
#include "velocity_component.hpp"

class GravitySystem
{

    public:

        GravitySystem(entt::registry& _Reg) : Reg_(_Reg) {}

        void calculateForces()
        {
            auto ViewPAB = Reg_.view<PositionComponent, AccelerationComponent, BodyComponent, GravitatorComponent>();

            for (auto e : ViewPAB)
            {
                auto& a = Reg_.get<AccelerationComponent>(e);
                a={{0.0, 0.0}};
            }

            for (auto e : ViewPAB)
            {
                const auto& p = ViewPAB.get<PositionComponent>(e);
                const auto& b = ViewPAB.get<BodyComponent>(e);
                auto& a = ViewPAB.get<AccelerationComponent>(e);

                // Add dummy component to start inner loop with next entity
                Reg_.emplace<DoneComponent_>(e);

                this->calculateInnerLoop(e, p.v, b.m, a.v);
            }

            Reg_.clear<DoneComponent_>();
        }

    private:

        void calculateInnerLoop(entt::entity _EntityOuter,
                                const Vec2Dd& _PosCompOuter,
                                double _BodyCompOuter,
                                Vec2Dd& _AccCompOuter)
        {
            auto View = Reg_.view<PositionComponent, AccelerationComponent, BodyComponent>(entt::exclude<DoneComponent_>);
            for (auto e : View)
            {
                const auto& p = View.get<PositionComponent>(e);
                const auto& b = View.get<BodyComponent>(e);
                auto& a = View.get<AccelerationComponent>(e);

                Vec2Dd Diff = (_PosCompOuter - p.v);
                double Rsqr = Diff.squaredNorm();

                if (Rsqr < 1.0e6) Rsqr = 1.0e6;

                Vec2Dd d = Diff / std::sqrt(Rsqr);

                constexpr double G = 6.6743e-11;
                Vec2Dd Tmp  = G / Rsqr * d;

                _AccCompOuter -= Tmp * b.m;
                a.v += Tmp * _BodyCompOuter;
            }
        }

        entt::registry& Reg_;

        // Dummy component to allow for nested loops that do not iterate all
        // objects from original view/group
        struct DoneComponent_{};

};

#endif // GRAVITY_SYSTEM_HPP
