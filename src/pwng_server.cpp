#include <iostream>

#include <entt/entity/registry.hpp>

#include "network_manager.hpp"


int main()
{
    entt::registry Reg;

    Reg.set<NetworkManager>();

    Reg.ctx<NetworkManager>().init();


    bool IsRunning = true;
    while (IsRunning)
    {
    }

    return EXIT_SUCCESS;
}
