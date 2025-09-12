#pragma once 

#include <assimp/scene.h>
#include <Utils.hpp>

#include <assimp/vector3.h>
#include <glm/glm.hpp>
using namespace glm;


glm::mat4 toGLM(const aiMatrix4x4& from);
glm::vec3 toGLM(const aiVector3D a);