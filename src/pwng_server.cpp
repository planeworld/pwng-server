#include <iostream>

#include <entt/entity/registry.hpp>
#include <concurrentqueue/concurrentqueue.h>

#include "network_manager.hpp"


int main()
{
    entt::registry Reg;
    moodycamel::ConcurrentQueue<std::string> InputQueue;
    moodycamel::ConcurrentQueue<std::string> OutputQueue;

    Reg.set<NetworkManager>();
    Reg.ctx<NetworkManager>().init(&InputQueue, &OutputQueue);

    bool IsRunning = true;
    while (IsRunning)
    {
        std::string Message;
        bool NewMessageFound = InputQueue.try_dequeue(Message);
        if (NewMessageFound)
            std::cout << Message << std::endl;
    }

    return EXIT_SUCCESS;
}
