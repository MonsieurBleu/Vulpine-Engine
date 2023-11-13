#ifndef TEAM_HPP
#define TEAM_HPP

#include <demos/Mage_Battle/Mage.hpp>

class Team : private std::list<MageRef>
{
    private : 
        uint healNB = 0;
        uint attackNB = 0;
        uint tankNB = 0;
        vec3 color;

    public : 

        static ModelRef healModel;
        static ModelRef attackModel;
        static ModelRef tankModel;
        static std::list<MageRef> allMages;

        void SpawnUnits(
            Scene& scene, 
            uint healNB, 
            uint attackNB, 
            uint tankNB, 
            vec3 spawnPoint,
            float spawnRadius, 
            vec3 color);

        void tick();
};

#endif