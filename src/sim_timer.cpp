#include "sim_timer.hpp"

void SimTimer::inc(const double& _Seconds)
{
    constexpr double S_PER_Y = 365.0*24.0*60.0*60.0;
    if (Active)
    {
        Seconds += _Seconds;
        while (Seconds >= S_PER_Y)
        {
            Seconds -= S_PER_Y;
            ++Years;
        }
    }
}

void SimTimer::start()
{
    Active = true;
    Seconds = 0.0;
    Years = 0u;
}

void SimTimer::stop()
{
    Active = false;
}

void SimTimer::toggle()
{
    if (Active)
    {
        this->stop();
    }
    else
    {
        this->start();
    }
}
