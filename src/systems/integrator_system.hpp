#ifndef INTEGRATOR_SYSTEM_HPP
#define INTEGRATOR_SYSTEM_HPP

#include <entt/entity/registry.hpp>

#include "acceleration_component.hpp"
#include "position_component.hpp"
#include "velocity_component.hpp"

class IntegratorSystem
{

    public:

        IntegratorSystem(entt::registry& _Reg) : Reg_(_Reg) {}

        void integrate(const double _Step) const
        {
            auto GroupAV = Reg_.group<AccelerationComponent>(entt::get<VelocityComponent>);
            for (auto e : GroupAV)
            {
                const auto& _a = GroupAV.get<AccelerationComponent>(e);
                auto& _v = GroupAV.get<VelocityComponent>(e);

                _v.v += _a.v * _Step;
            }
            auto GroupVP = Reg_.group<VelocityComponent, PositionComponent>();
            for (auto e : GroupVP)
            {
                const auto& _v = GroupVP.get<VelocityComponent>(e);
                auto& _p = GroupVP.get<PositionComponent>(e);

                _p.v += _v.v * _Step;
            }
        }

    private:

       entt::registry& Reg_;

};

#endif // INTEGRATOR_SYSTEM_HPP
