#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstring>

#include <Graphics/Animation.hpp>

using namespace glm;

/*
    .vulpineMesh :
        ==> Triangles only

        header

        if elements :
            3D uint cache optimized faces of size facesCount, in [0-erticesCount] range

        3D positions of size VerticesCount, in the AABB range
        3D normals of size VerticesCount, in normalize range
        2D UVs of size VerticesCount, in [0-1] range

        if skinned
            4D weights in [0-1] range
            4D uint weights ID in [0-bonesCount] range
*/

enum VEAC_EXPORT_FORMAT
{
    FORMAT_DEFAULT,
    FORMAT_SANCTIA
};

enum VulpineMesh_Type
{
    ELEMENTS ,
    ELEMENTS_SKINNED ,
    ARRAY ,
    ARRAY_SKINNED 
};

enum VulpineMesh_VertexPacking : uint8
{
    DEFAULT_PACKING, /* FLOAT POS, NORMAL, UV*/
    COMPRESSED_UVEC4_PACKING /* MERGED POS & NORMAL, + 32 bit STENCIL */
};

struct VulpineMesh_HeaderBase
{
    VulpineMesh_Type type = ELEMENTS;
    uint facesCount;
    uint VerticesCount;

    vec3 AABBmin;
    vec3 AABBmax;

    VulpineMesh_VertexPacking vertexPacking = DEFAULT_PACKING;
};

struct VulpineMesh_Header : VulpineMesh_HeaderBase
{
    const uint stencil[32 - sizeof(VulpineMesh_HeaderBase)/4] = {0};
};

/*
    .vulpineSkeleton

        header

        1D VulpineSkeleton_Bone of size bonesCount

        if named bones :
            1D strings with offset of size MAX_BONES_NAME
*/

#define MAX_BONES_NAME 256

enum VulpineSkeleton_Type
{
    FAST,
    NAMED_BONES
};

struct VulpineSkeleton_Header
{
    uint version = 1;
    uint type = 0;
    uint bonesCount = 0;
    uint stencil[29];
};

struct VulpineSkeleton_Bone
{
    mat4 t = mat4(1);
    int parent = 0;
    int children[15] = {0};
};

// typedef uint8_t byte;
// struct AnimationFileHeader
// {
//     const char magic[4] = {'V', 'A', 'N', 'I'};
//     char animationName[128];
//     float duration;
//     unsigned int totalBoneNumber;
//     unsigned int animatedBoneNumber;

//     AnimationFileHeader(const char *name, float duration, unsigned int totalBoneNumber, unsigned int animatedBoneNumber)
//         : duration(duration), totalBoneNumber(totalBoneNumber), animatedBoneNumber(animatedBoneNumber)
//     {
//         strncpy(animationName, name, 127);
//     }
// };

// enum AnimationBehaviour : byte
// {
//     /** The value from the default node transformation is taken*/
//     AnimationBehaviour_DEFAULT = 0x0,

//     /** The nearest key value is used without interpolation */
//     AnimationBehaviour_CONSTANT = 0x1,

//     /** The value of the nearest two keys is linearly
//      *  extrapolated for the current time value.*/
//     AnimationBehaviour_LINEAR = 0x2,

//     /** The animation is repeated.
//      *
//      *  If the animation key go from n to m and the current
//      *  time is t, use the value at (t-n) % (|m-n|).*/
//     AnimationBehaviour_REPEAT = 0x3,
// };

// struct AnimationBoneData
// {
//     unsigned int boneID;
//     AnimationBehaviour preStateBehaviour;
//     AnimationBehaviour postStateBehaviour;
//     unsigned int keyframeNumber;
// };

// struct AnimationKeyframeData
// {
//     float time;
//     vec3 translation;
//     quat rotation;
//     vec3 scale;
// };