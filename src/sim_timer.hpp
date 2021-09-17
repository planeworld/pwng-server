#ifndef SIM_TIMER_HPP
#define SIM_TIMER_HPP

#include <cmath>
#include <cstdint>
#include <string>

class SimTimer
{
    
    public:
        
        std::uint32_t getYears() const;
        std::uint32_t getDaysFraction() const;
        std::uint32_t getHoursFraction() const;
        std::uint32_t getMinutesFraction() const;
        double        getSecondsFraction() const;
        double        getSeconds() const;
        double        getAcceleration() const;
        std::string   toStamp() const;
        
        bool isActive() const;

        void fromStamp(const std::string& _s);
        void inc(const double& _Seconds);
        void setAcceleration(double _a);
        void start();
        void stop();
        void toggle();

    private:
        
        bool            Active_{false};
        double          Acceleration_{1.0};
        double          Seconds_{0.0};
        std::uint32_t   Years_{0u};
};

inline std::uint32_t SimTimer::getYears() const
{
    return Years_;
}

inline double SimTimer::getSecondsFraction() const
{
    constexpr double M_PER_S = 1.0/60.0;
    constexpr double S_PER_M = 60.0;
    return ((Seconds_ * M_PER_S) - std::floor(Seconds_ * M_PER_S)) * S_PER_M;
}

inline std::uint32_t SimTimer::getMinutesFraction() const
{
    constexpr double M_PER_H = 60.0;
    constexpr double H_PER_S = 1.0/(60.0*M_PER_H);
    return static_cast<uint32_t>(((Seconds_ * H_PER_S) - std::floor(Seconds_ * H_PER_S)) * M_PER_H);
}

inline std::uint32_t SimTimer::getHoursFraction() const
{
    constexpr double H_PER_D = 24.0;
    constexpr double D_PER_S = 1.0/(60.0*60.0*H_PER_D);
    return static_cast<uint32_t>(((Seconds_ * D_PER_S) - std::floor(Seconds_ * D_PER_S)) * H_PER_D);
}

inline std::uint32_t SimTimer::getDaysFraction() const
{
    constexpr double D_PER_Y = 365.0;
    constexpr double Y_PER_S = 1.0/(60.0*60.0*24.0*D_PER_Y);
    return static_cast<uint32_t>(((Seconds_ * Y_PER_S) - std::floor(Seconds_ * Y_PER_S)) * D_PER_Y);
}

inline double SimTimer::getSeconds() const
{
    return Seconds_;
}

inline double SimTimer::getAcceleration() const
{
    return Acceleration_;
}

inline std::string SimTimer::toStamp() const
{
    return std::to_string(Years_) + ":" + std::to_string(Seconds_);
}

inline bool SimTimer::isActive() const
{
    return Active_;
}

inline void SimTimer::setAcceleration(double _a)
{
    Acceleration_ = _a;
}

#endif // SIM_TIMER_HPP
