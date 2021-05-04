#ifndef NAME_COMPONENT_HPP
#define NAME_COMPONENT_HPP

#include <cstring>

// 31 characters and trailing delimiter (\0)
constexpr std::size_t NAME_SIZE_MAX = 32;

struct NameComponent
{
    // Use fixed length char[] to ensure memory is aligned and not dynamically
    // allocated (in comparison to std::string). Hence, a name-system is used
    // to set names via string copy and test for length
    char Name[NAME_SIZE_MAX]{"Unknown"};
};
#endif // NAME_COMPONENT_HPP
