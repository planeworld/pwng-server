#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <atomic>
#include <iostream>
#include <sstream>
#include <string>

class ErrorHandler
{
    
    public:
        
        bool checkError()
        {
            return ErrorFlag.exchange(false);
        }
        
        void report(const std::string& _Message)
        {
            // std::cerr is thread safe, but messages could be mangled
            // Therefore, the message is formed before dispatching
            std::stringstream Message;
            Message << "[e] " << _Message;
            std::cerr << Message.str() << std::endl;
            ErrorFlag = true;
        }
    
    private:
        
        std::atomic_bool ErrorFlag{false};
        
};

#endif // ERROR_HANDLER_H
