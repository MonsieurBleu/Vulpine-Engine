#pragma once

#include <glm/glm.hpp>

struct Pathfind
{
    bool initialized = false;
    glm::vec3 lastPos = glm::vec3(0);

    uint32_t navmeshBuildPriority = 0; 
    
    glm::vec3 target;
    bool hasTarget = false;
};


void ManageNavGarbage();