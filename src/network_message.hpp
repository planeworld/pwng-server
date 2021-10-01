#ifndef NETWORK_MESSAGE_HPP
#define NETWORK_MESSAGE_HPP

#include <entt/entity/entity.hpp>
#include <rapidjson/document.h>

#include <string>

enum class NetworkMessageClassificationType : int
{
    INVALID,
    CMD, // Command
    EVT, // Subscription, event-based
    S01, // Subscription, each 0.1s, 10.0 Hz
    S05, // Subscription, each 0.5s, 5.0 Hz
    S1,  // Subscription, each 1.0s, 1.0 Hz
    S5,  // Subscription, each 5.0s, 0.2 Hz
    S10  // Subscription, each 10.0s, 0.1 Hz
};

// JSON message
struct NetworkMessage
{
    entt::entity ClientID;
    std::string Payload;
};

// JSON message parsed into rapidjson::Document
struct NetworkMessageParsed
{
    entt::entity ClientID;
    std::shared_ptr<rapidjson::Document> Payload;
};

// JSON message classified by JSONRPC message type (command, subscription,...)
struct NetworkMessageClassified
{
    entt::entity ClientID;
    NetworkMessageClassificationType Class;
    std::shared_ptr<rapidjson::Document> Payload;
};

#endif // NETWORK_MESSAGE_HPP
