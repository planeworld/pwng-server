#ifndef GALAXY_MANAGER_HPP
#define GALAXY_MANAGER_HPP

#include <entt/entity/registry.hpp>

#include "name_system.hpp"

class GalaxyManager
{

    public:

        explicit GalaxyManager(entt::registry& _Reg) : Reg_(_Reg),
                                                       SysName_(_Reg){}
        ~GalaxyManager(){}

        void generateGalaxy();

    private:

        entt::registry&  Reg_;
        NameSystem       SysName_;
};


#endif // GALAXY_MANAGER_HPP
