#ifndef NETWORK_MESSAGE_HPP
#define NETWORK_MESSAGE_HPP

#include <cstdint>
#include <string>

using ConIDType = std::uint64_t;

struct NetworkMessage
{
    ConIDType ID;
    std::string Payload;
};

#endif // NETWORK_MESSAGE_HPP
