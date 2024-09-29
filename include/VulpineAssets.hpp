#pragma once

#include <Mesh.hpp>

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
    uint verticesCount;

    vec3 AABBmin;
    vec3 AABBmax;

    VulpineMesh_VertexPacking vertexPacking = DEFAULT_PACKING;
};

struct VulpineMesh_Header : VulpineMesh_HeaderBase
{
    const uint stencil[32 - sizeof(VulpineMesh_HeaderBase)/4] = {0};
};

MeshVao loadVulpineMesh(const char *filename);

struct VulpineSkeleton_Header
{
    uint version = 1;
    uint type = 0;
    uint bonesCount = 0;
    uint stencil[29];
};


