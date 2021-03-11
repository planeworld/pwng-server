#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <atomic>
#include <cassert>
#include <iostream>
#include <string>
#include <sstream>

// Only compile debug blocks in debug mode
#ifdef NDEBUG
    #define DBLK(x)
#else
    #define DBLK(x) x
#endif

class MessageHandler
{
    
    public:
        
        typedef enum
        {
            INFO = 0,
            DEBUG_L1 = 1,
            DEBUG_L2 = 2,
            DEBUG_L3 = 3
        } ReportLevelType;
        
        void report(const std::string& _Source, const std::string& _Message, const ReportLevelType _Level = INFO)
        {
            if (_Level == INFO)
            {
                std::stringstream Message;
                Message << "[ I  ][ " << _Source << " ] " <<  _Message;
                std::cout << Message.str() << std::endl;
            }
            else
            {
                if (_Level <= Level_)
                {
                    std::stringstream Message;
                    Message << "[ D" << _Level << " ][ " << _Source << " ] " << _Message;
                    std::cout << Message.str() << std::endl;
                }
            }
        }

        void reportRaw(const std::string& _Message, const ReportLevelType _Level = INFO)
        {
            if (_Level <= Level_)
            {
                std::cout << _Message;
            }
        }

        void setLevel(ReportLevelType _Level) {Level_ = _Level;}

    private:

        std::atomic<ReportLevelType> Level_{DEBUG_L3};

};

#endif // MESSAGE_HANDLER_H
