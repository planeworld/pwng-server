#ifndef NAME_SYSTEM_HPP
#define NAME_SYSTEM_HPP

#include <string>
#include <unordered_map>

#include <entt/entity/registry.hpp>

#include "name_component.hpp"

class NameSystem
{

    public:

        explicit NameSystem(entt::registry& _Reg) : Reg_(_Reg) {}

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
            MapToEntityId_.insert({_Name, entt::to_integral(_e)});
        }

    private:

        entt::registry& Reg_;

        std::unordered_map<std::string, entt::id_type> MapToEntityId_;

};

#endif // NAME_SYSTEM_HPP
