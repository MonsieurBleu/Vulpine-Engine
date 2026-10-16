#pragma once

#include <glm/glm.hpp>

struct AgentNavigation
{
    bool initialized = false;
    glm::vec3 lastPos = glm::vec3(0);

    uint8_t navmeshBuildPriority = 0;
};


void ManageNavGarbage();