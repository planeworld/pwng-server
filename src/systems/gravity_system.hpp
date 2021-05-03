#ifndef GRAVITY_SYSTEM_HPP
#define GRAVITY_SYSTEM_HPP

#include <iostream>

#include <entt/entity/registry.hpp>

#include "acceleration_component.hpp"
#include "body_component.hpp"
#include "math_types.hpp"
#include "position_component.hpp"
#include "sim_components.hpp"
#include "velocity_component.hpp"

class GravitySystem
{

    public:

        GravitySystem(entt::registry& _Reg) : Reg_(_Reg) {}

        void calculateForces()
        {
            Reg_.view<AccelerationComponent>().each
            (
                [](auto _e, auto& _a)
                {
                    _a.v = {0.0, 0.0};
                }
            );
            Reg_.view<StarSystemComponent>().each(
                [this](auto _e, const auto& _StarSystem)
                {
                    for (auto i=0u; i<_StarSystem.Objects.size(); ++i)
                    {
                        auto& a_i = Reg_.get<AccelerationComponent>(_StarSystem.Objects[i]);
                        const auto& b_i = Reg_.get<BodyComponent>(_StarSystem.Objects[i]);
                        const auto& p_i = Reg_.get<PositionComponent>(_StarSystem.Objects[i]);

                        for (auto j=i+1; j<_StarSystem.Objects.size(); ++j)
                        {
                            auto& a_j = Reg_.get<AccelerationComponent>(_StarSystem.Objects[j]);
                            const auto& b_j = Reg_.get<BodyComponent>(_StarSystem.Objects[j]);
                            const auto& p_j = Reg_.get<PositionComponent>(_StarSystem.Objects[j]);

                            Vec2Dd Diff = (p_i.v - p_j.v);
                            double Rsqr = Diff.squaredNorm();

                            if (Rsqr < 1.0e6) Rsqr = 1.0e6;

                            Vec2Dd d = Diff / std::sqrt(Rsqr);

                            constexpr double G = 6.6743e-11;
                            Vec2Dd Tmp  = G / Rsqr * d;

                            a_i.v -= Tmp * b_j.m;
                            a_j.v += Tmp * b_i.m;
                        }
                    }
                }
            );
        }

    private:

        entt::registry& Reg_;

};

#endif // GRAVITY_SYSTEM_HPP
