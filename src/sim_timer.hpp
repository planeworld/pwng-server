#ifndef SIM_TIMER_HPP
#define SIM_TIMER_HPP

#include <cstdint>
#include <string>

class SimTimer
{
    
    public:
        
        std::uint32_t getYears() const;
        double        getSeconds() const;
        std::string   toStamp() const;
        
        bool isActive() const;
        
        void inc(const double& _Seconds);
        void start();
        void stop();
        void toggle();

    private:
        
        bool            Active{false};
        double          Seconds{0.0};
        std::uint32_t   Years{0u};
};

inline std::uint32_t SimTimer::getYears() const
{
    return Years;
}

inline double SimTimer::getSeconds() const
{
    return Seconds;
}

inline std::string SimTimer::toStamp() const
{
    return std::to_string(Years) + ":" + std::to_string(Seconds);
}

inline bool SimTimer::isActive() const
{
    return Active;
}

#endif // SIM_TIMER_HPP
