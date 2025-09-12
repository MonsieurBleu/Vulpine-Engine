#pragma once 

/* 
 * Stencils Types are used to temporarly hold important file 
 * informations for sorting, processing and/or optimization
 * before being converted to Vulpine format for on output 
 * files.
*/

#include <vector>
#include <unordered_map>
#include <assimp/scene.h>
#include <glm/glm.hpp>
using namespace glm;

struct STENCIL_BaseMeshInfos
{
    uint facesCount;
    uint verticesCount;

    /* Need to be freed manually */
    ivec3 *faces = nullptr;
    
    /* DO NOT FREE : Points to the assimp scene data */
    vec3  *positions = nullptr;
    
    //* DO NOT FREE : Points to the assimp scene data */
    vec3  *normals = nullptr;

    /* Need to be freed manually */
    vec2  *uvs = nullptr;

    std::vector<uvec4> packedinfos;
};

struct STENCIL_MeshInfos : STENCIL_BaseMeshInfos
{
    /* Need to be freed manually */
    vec4 *weights = nullptr;

    /* Need to be freed manually */
    ivec4 *weightsID = nullptr;
};

struct Stencil_BoneInfos
{
    int id;
    int parentID = 0; // TODO : remove
    aiNode *node = nullptr;
};

typedef  std::unordered_map<std::string, Stencil_BoneInfos> Stencil_BoneMap;
