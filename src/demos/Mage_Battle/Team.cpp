#include <demos/Mage_Battle/Team.hpp>
#include <random>

ModelRef Team::healModel;
ModelRef Team::attackModel;
ModelRef Team::tankModel;
std::list<MageRef> Team::allMages;

void Team::SpawnUnits(
    Scene& scene, 
    uint healNB, 
    uint attackNB, 
    uint tankNB, 
    vec3 spawnPoint,
    float spawnRadius, 
    vec3 color)
{
    this->color = color;

    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_real_distribution<float> distr(-spawnRadius, spawnRadius);

    for(uint i = 0; i < healNB; i++)
    {
        MageRef newMage = SpawnNewMage(
            allMages,
            scene,
            healModel, 
            color, 
            spawnPoint + vec3(distr(generator), 0.f, distr(generator)), 
            HEAL);
        push_back(newMage);
        allMages.push_back(newMage);
    }
    
    for(uint i = 0; i < attackNB; i++)
    {
        MageRef newMage = SpawnNewMage(
            allMages,
            scene,
            attackModel, 
            color, 
            spawnPoint + vec3(distr(generator), 0.f, distr(generator)), 
            ATTACK);
        push_back(newMage);
        allMages.push_back(newMage);
    }

    for(uint i = 0; i < tankNB; i++)
    {
        MageRef newMage = SpawnNewMage(
            allMages,
            scene,
            tankModel, 
            color, 
            spawnPoint + vec3(distr(generator), 0.f, distr(generator)), 
            TANK);
        push_back(newMage);
        allMages.push_back(newMage);
    }

    this->healNB += healNB;
    this->attackNB+= attackNB;
    this->tankNB += tankNB;
}

void Team::tick()
{
    for(auto i : *this)
    {
        i->tick();
    }
}
