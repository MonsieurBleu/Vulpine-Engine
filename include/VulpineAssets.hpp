#pragma once

#include <Mesh.hpp>

enum VulpineMesh_Type
{
    ELEMENTS = 0, 
    ELEMENTS_SKINNED = 1,
    ARRAY = 2,
    ARRAY_SKINNED = 2
};

struct VulpineMesh_Header
{
    VulpineMesh_Type type = ELEMENTS;
    uint facesCount;
    uint verticesCount;

    vec3 AABBmin;
    vec3 AABBmax;

    uint stencil[23];
};

MeshVao loadVulpineMesh(const std::string &filename);

struct VulpineSkeleton_Header
{
    uint version = 1;
    uint type = 0;
    uint bonesCount = 0;
    uint stencil[29];
};


