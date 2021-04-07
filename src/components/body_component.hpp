#ifndef BODY_COMPONENT_HPP
#define BODY_COMPONENT_HPP

struct BodyComponent
{
    double m{1.0}; // mass
    double i{1.0}; // inertia
};

struct TemperatureComponent
{
    double h{1.0};
};

struct GravitatorComponent{};

#endif // BODY_COMPONENT_HPP
