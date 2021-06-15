#ifndef NETWORK_MESSAGE_HPP
#define NETWORK_MESSAGE_HPP

#include <entt/entity/entity.hpp>

#include <string>

struct NetworkMessage
{
    entt::entity ClientID;
    std::string Payload;
};

#endif // NETWORK_MESSAGE_HPP
