#ifndef MAGE_H
#define MAGE_H

#include <Globals.hpp>
#include <Mesh.hpp>
#include <Scene.hpp>
#include <Helpers.hpp>

enum MageType
{
    NONE,
    DEBUG,
    HEAL,
    ATTACK,
    TANK
};

#define MageRef std::shared_ptr<Mage>
#define SpawnNewMage std::make_shared<Mage>

#define MAGE_MAX_HP 100

#define ARENA_RADIUS 30.f

class Mage{
	protected :	
        SceneTubeLight magicRayLight;
        std::shared_ptr<TubeLightHelper> magicRay;
        bool magicRayInScene = false;

		ModelRef model;
		vec3 color;
        float hue;
		MageType type; 
        
        float HP = 100;
        bool alive = true;

        Scene &scene;

        std::list<MageRef> &allMages;

        void addRayToScene();
        void removeRayFromScene();

        void initiateTarget(Mage *mage);
        void resetTarget();
        Mage* findClosestVisibleEnemy();
        Mage* findClosestVisibleInjuredAlly();
        Mage* findClosestVisibleMenacedAlly();

        vec3 fleeFromMenace(vec3 currentDir);

        vec3 menacePosition;
        float timeSinceLastMenace = 0;
        bool applyDamageNextTurn = false;
        bool applyHealNextTurn = false;

        void menace(Mage* mage);
        void heal(Mage* mage);

	public : 

		Mage(std::list<MageRef> &allMages, Scene &scene, ModelRef model, vec3 color, vec3 position, MageType type);
		void dead(); 
		void tick();		
		bool isDead();

        MageType getType();
        ModelRef getModel();
};

#endif