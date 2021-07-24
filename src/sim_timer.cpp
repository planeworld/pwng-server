#include "sim_timer.hpp"

void SimTimer::fromStamp(const std::string& _s)
{
    auto p = _s.find(':');
    if (p != std::string::npos)
    {
        Years_ = std::stoul(_s.substr(0, p), nullptr);
        Seconds_ = std::stod(_s.substr(p+1, std::string::npos));
    }
    else
    {
        // error
    }
}

void SimTimer::inc(const double& _Seconds)
{
    constexpr double S_PER_Y = 365.0*24.0*60.0*60.0;
    if (Active_)
    {
        Seconds_ += _Seconds;
        while (Seconds_ >= S_PER_Y)
        {
            Seconds_ -= S_PER_Y;
            ++Years_;
        }
    }
}

void SimTimer::start()
{
    Active_ = true;
    Seconds_ = 0.0;
    Years_ = 0u;
}

void SimTimer::stop()
{
    Active_ = false;
}

void SimTimer::toggle()
{
    if (Active_)
    {
        this->stop();
    }
    else
    {
        this->start();
    }
}
