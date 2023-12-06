#include <demos/Mage_Battle/Mage.hpp>
#include <Globals.hpp>

float getHue(float red, float green, float blue) {

    float min = red < green ? red : green;
    min = min < blue ? min : blue;

    float max = red > green ? red : green;
    max = max > blue ? max : blue;

    if (min == max) {
        return 0.f;
    }


    float hue = 0.f;
    if (max == red) {
        hue = (green - blue) / (max - min);

    } else if (max == green) {
        hue = 2.f + (blue - red) / (max - min);

    } else {
        hue = 4.f + (red - green) / (max - min);
    }

    return hue/6.0;
}

Mage::Mage(
    std::list<MageRef> &allMages,
    Scene &scene, 
    ModelRef model, 
    vec3 color, 
    vec3 position, 
    MageType type) : scene(scene), allMages(allMages)
{
	this->model = model->copyWithSharedMesh();
	this->color = color;
	this->type = type;

	this->model->state.setPosition(position);
    this->model->state.setScale(model->state.scale);

    this->model->uniforms.add(ShaderUniform(&this->color, 32));
    this->model->uniforms.add(ShaderUniform(&this->HP, 33));
    this->model->uniforms.add(ShaderUniform((int *)&this->type, 34));

    scene.add(this->model);

    magicRayLight = newTubetLight();
    vec3 lightColor = type == HEAL ? vec3(1.5)-color : color;
    lightColor = type == TANK ? vec3(0.25)*color : lightColor;
    magicRayLight->setIntensity(0.5).setRadius(3.0).setColor(lightColor);

    magicRay = std::make_shared<TubeLightHelper>(magicRayLight);
    magicRayLight->setPos(this->model->state.position, this->model->state.position + vec3(0, 5, 0));

    hue = getHue(color.r, color.g, color.b);
    hue = -0.1 + hue*0.2;

    std::cout << model->state.scale.x << "\n";

    // this->model->state.setPosition(
    //     this->model->state.position + 
    //     vec3(0, 1.91*model->state.scale.x, 0));

    vec2 pdir = normalize(vec2(-position.x, -position.z));
    this->model->state.setRotation(vec3(
        0, 
        atan2(pdir.x, pdir.y), 
        0
    ));
}

void Mage::dead(){
    if(alive)
    {
        scene.remove(this->model);
        removeRayFromScene();
        
        for(auto i = allMages.begin(); i != allMages.end(); i++)
            if(i->get() == this)
                allMages.erase(i);

        alive = false;
    }
}

bool Mage::isDead()
{
    return !alive;
}

ModelRef Mage::getModel()
{
    return model;
}

void Mage::addRayToScene()
{
    if(!magicRayInScene)
    {
        scene.add(magicRay);
        scene.add(magicRayLight);
        magicRayInScene = true;
    }
}

void Mage::removeRayFromScene()
{
    if(magicRayInScene)
    {
        scene.remove(magicRay);
        scene.remove(magicRayLight);
        magicRayInScene = false;
    }
}

Mage* Mage::findClosestVisibleEnemy()
{
    Mage* res = NULL;
    float minDist = 1E9;

    for(auto i : allMages)
        if(i->hue != hue && !i->applyDamageNextTurn)
        {
            vec3 v = model->state.position-i->getModel()->state.position;
            float dist = v.x*v.x + v.y*v.y + v.z*v.z;
            res = dist < minDist ? i.get() : res;
            minDist = dist < minDist ? dist : minDist;
        }

    return res;
}

Mage* Mage::findClosestVisibleInjuredAlly()
{
    Mage* res = NULL;
    float minDist = 1E9;

    for(auto i : allMages)
        if(i->hue == hue
        && (i->HP < MAGE_MAX_HP || i->applyDamageNextTurn) 
        && !i->applyHealNextTurn
        && i.get() != this)
        {
            vec3 v = model->state.position-i->getModel()->state.position;
            float dist = v.x*v.x + v.y*v.y + v.z*v.z;
            res = dist < minDist ? i.get() : res;
            minDist = dist < minDist ? dist : minDist;
        }

    return res;
}

Mage* Mage::findClosestVisibleMenacedAlly()
{
    Mage* res = NULL;
    float minDist = 1E9;

    float time = globals.unpausedTime.getElapsedTime();

    for(auto i : allMages)
        if(i->hue == hue
          && i->type != TANK
          && (time - i->timeSinceLastMenace) < 0.5 
          && i.get() != this)
        {
            vec3 v = model->state.position-i->getModel()->state.position;
            float dist = v.x*v.x + v.y*v.y + v.z*v.z;
            res = dist < minDist ? i.get() : res;
            minDist = dist < minDist ? dist : minDist;
        }

    return res;
}



void Mage::initiateTarget(Mage *mage)
{
    magicRayLight->setPos(
        mage->getModel()->state.position + vec3(0, hue, 0), 
        model->state.position + vec3(0, hue, 0));
    addRayToScene();

    if(type == ATTACK)
        mage->menace(this);
    else if(type == HEAL)
        mage->heal(this);
}

void Mage::resetTarget()
{
    removeRayFromScene();
}

void Mage::menace(Mage *mage)
{
    menacePosition = mage->getModel()->state.position;
    timeSinceLastMenace = globals.unpausedTime.getElapsedTime();
    applyDamageNextTurn = true;
}

void Mage::heal(Mage *mage)
{
    applyHealNextTurn = true;
}

vec3 Mage::fleeFromMenace(vec3 currentDir)
{
    float menaceMemory = 0.1;

    // if(timeSinceLastMenace)
    // std::cout << timeSinceLastMenace << " " << globals.unpausedTime.getElapsedTime() << "\n";

    if(timeSinceLastMenace && globals.unpausedTime.getElapsedTime() - timeSinceLastMenace < menaceMemory)
    {
        vec3 dirToMenace = normalize(menacePosition - model->state.position);
        normalize(currentDir);
        vec3 result = currentDir;

        double d = dot(dirToMenace, currentDir);

        if(d < 0.0) {
            result = currentDir;
        } else {
            result = -dirToMenace;
        }

        return result;
    }

    return currentDir;
}

MageType Mage::getType(){
    return this->type;
}

void Mage::tick()
{
    if(isDead()) return;
    if(globals.unpausedTime.isPaused()) return;

    float deltaTime = globals.unpausedTime.getDelta();

    if(applyHealNextTurn)
    {
        const float HealPS = 10;
        HP += HealPS*deltaTime;
        HP = HP > MAGE_MAX_HP ? MAGE_MAX_HP : HP;
        applyHealNextTurn = false;
    }

    if(applyDamageNextTurn)
    {
        const float AttacksDPS = 20;
        const float dmgMigitation = type == TANK ? 0.6f : 1.f;
        HP -= AttacksDPS*deltaTime*dmgMigitation;

        applyDamageNextTurn = false;

        if(HP <= 0.f)
        {
            dead();
            return;
        }
    }   

    resetTarget();

    const float speedMpS  = 1.0; 
    float speed = speedMpS*deltaTime;
    vec3 deplacementDir(0);

	switch(type)
    {
        // case DEBUG : break;
        case ATTACK : 
        {
            const float rayRange = 7.5;
            const float minDistFromEnemy = rayRange*0.4;

            Mage *target = findClosestVisibleEnemy();

            if(target)
            {
                float dist = length(model->state.position - target->getModel()->state.position);
                deplacementDir = normalize(target->getModel()->state.position - model->state.position);

                if(dist < rayRange)
                {
                    initiateTarget(target);

                    deplacementDir = dist < minDistFromEnemy ? -deplacementDir : deplacementDir;
                }
            }
        }
            break;



        case HEAL : 
        {
            speed *= 1.5;
            const float rayRange = 3.0;
            const float minDistFromEnemy = rayRange*0.8;

            Mage *target = findClosestVisibleInjuredAlly();

            if(target)
            {
                float dist = length(model->state.position - target->getModel()->state.position);
                deplacementDir = normalize(target->getModel()->state.position - model->state.position);

                if(dist < rayRange)
                {
                    initiateTarget(target);

                    deplacementDir = dist < minDistFromEnemy ? -deplacementDir : deplacementDir;
                }
            }

            deplacementDir = fleeFromMenace(deplacementDir);
        }
        break;

        case TANK :
        {
            const float distFromProtect = 1.0;
            Mage *target = findClosestVisibleMenacedAlly();

            if(target)
            {
                vec3 dest = target->getModel()->state.position;
                dest -= normalize(dest-target->menacePosition)*distFromProtect;
                deplacementDir = normalize(dest-model->state.position);
                // initiateTarget(target);
            }
        }
        break;

        default:
        {
            // float minDist = 1E9;
            // vec3 dest(0);
            // for(auto i : allMages)
            //     if(i->color.x != color.x && i->color.y != color.y && i->color.z != color.z)
            //     // if(i.get() != this)
            //     {
            //         float dist = length(model->state.position-i->getModel()->state.position);
                    
            //         if(dist > 0.1 && dist < minDist)
            //         {
            //             dest = i->getModel()->state.position;
            //             minDist = dist;
            //         }
            //     }
            
            // if(minDist <= 0.15)
            //     dead();
            // else
            // {
            //     // addRayToScene();
            //     // magicRayLight->setPos(dest, model->state.position);
            //     deplacementDir = normalize(dest-model->state.position);
            // } 
        }

            break;
	}

    deplacementDir.y = 0.f;

    model->state.setPosition(model->state.position + (deplacementDir * speed));

    model->state.setPosition(
        clamp(model->state.position, 
        vec3(-ARENA_RADIUS, -1E6, -ARENA_RADIUS), vec3(ARENA_RADIUS, 1E6, ARENA_RADIUS)));
    
    model->state.setRotation(vec3(
        0, 
        atan2(deplacementDir.x, deplacementDir.z), 
        0
    ));
 
}