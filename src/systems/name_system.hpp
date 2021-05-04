#ifndef NAME_SYSTEM_HPP
#define NAME_SYSTEM_HPP

#include <entt/entity/registry.hpp>

#include "name_component.hpp"

class NameSystem
{

    public:

        NameSystem(entt::registry& _Reg) : Reg_(_Reg) {}

        void setName(entt::entity _e, const std::string& _Name)
        {
            auto& CompName = Reg_.emplace_or_replace<NameComponent>(_e);
            if (_Name.size() > NAME_SIZE_MAX-1)
            {
                auto Name = _Name;
                Name.resize(NAME_SIZE_MAX-1);
                strcpy(CompName.Name, Name.c_str());
            }
            else
            {
                strcpy(CompName.Name, _Name.c_str());
            }

        }

    private:

        entt::registry& Reg_;

};

#endif // NAME_SYSTEM_HPP
