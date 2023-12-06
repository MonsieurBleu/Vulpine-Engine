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
    scenePTR = &scene;

    this->color = color;

    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
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
    if(!attackNB)
    {
        std::vector<MageRef> newMages;
        int nbSpawnHeal = healNB/2 + 1;
        int nbSpawnTank = tankNB/2 + 1;
        // std::cout << nbSpawnHeal << " " << nbSpawnHeal << "\n";

        for(auto i : *this)
        {
            if( !i->isDead() && (
                (nbSpawnHeal && i->getType() == MageType::HEAL) ||
                (nbSpawnTank && i->getType() == MageType::TANK)
            ))
            {
                if(i->getType() == MageType::HEAL)
                    nbSpawnHeal--;
                
                if(i->getType() == MageType::TANK)
                    nbSpawnTank--;

                MageRef newMage = SpawnNewMage(
                    allMages, *scenePTR, attackModel, color, 
                    i->getModel()->state.position, MageType::ATTACK);
                
                newMages.push_back(newMage);
                i->dead();
            }            
        }

        for(auto i : newMages)
        {
            allMages.push_back(i);
            push_back(i);
        }
    }


    healNB = 0;
    attackNB = 0;
    tankNB = 0;

    for(auto i : *this)
    {
        if(!i->isDead())
            switch (i->getType())
            { 
            case MageType::ATTACK : attackNB++; break;
            case MageType::HEAL : healNB++; break;
            case MageType::TANK : tankNB++; break;
            default: break;
            }

        i->tick();
    }

    // healNB = globals.appTime.getElapsedTime();

    //std::cout << attackNB << "\n";
}

void Team::setMenu(FastUI_valueMenu &menu, std::u32string name)
{
    menu.push_back(
        {FastUI_menuTitle(menu.ui, name), FastUI_valueTab(menu.ui, {
            FastUI_value((const int*)&healNB,   U"Priest\t", U" units"),
            FastUI_value((const int*)&tankNB,   U"Tank\t", U" units"),
            FastUI_value((const int*)&attackNB, U"Mage\t", U" units"),
        })}
    );
}

