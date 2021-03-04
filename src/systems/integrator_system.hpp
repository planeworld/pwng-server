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
            Reg_.group<PositionComponent<double>,
                       VelocityComponent<double>,
                       AccelerationComponent<double>>().each
                ([_Step](auto _e, auto& _p, auto& _v, auto& _a)
                {
                    _v.x += _a.x * _Step;
                    _v.y += _a.y * _Step;
                    _p.x += _v.x * _Step;
                    _p.y += _v.y * _Step;
                });

        }

    private:

       entt::registry& Reg_;

};

#endif // INTEGRATOR_SYSTEM_HPP
