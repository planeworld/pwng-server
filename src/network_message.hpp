#ifndef NETWORK_MESSAGE_HPP
#define NETWORK_MESSAGE_HPP

#include <entt/entity/entity.hpp>
#include <rapidjson/document.h>

#include <string>

// JSON message
struct NetworkMessage
{
    entt::entity ClientID;
    std::string Payload;
};

// JSON message parsed into rapidjson::Document
struct NetworkDocument
{
    entt::entity ClientID;
    std::shared_ptr<const rapidjson::Document> Payload;
};

#endif // NETWORK_MESSAGE_HPP
