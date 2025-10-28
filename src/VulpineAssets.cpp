#include <VulpineAssets.hpp>
#include <Graphics/Skeleton.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <Utils.hpp>
#include <iostream>
#include <fstream>

GenericSharedBuffer getChunk(std::ifstream &file, uint size)
{   
    GenericSharedBuffer buff(new char[size]);
    file.read(buff.get(), size);
    return buff;
};

MeshVao loadVulpineMesh(const char *filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if(!file)
    {
        FILE_ERROR_MESSAGE(filename, strerror(errno) << "! The loader will return an empty object.")
        return MeshVao();
    }

    VulpineMesh_Header h;

    file.read((char *)&h, sizeof(VulpineMesh_Header));

    const float v1size = h.verticesCount*sizeof(float);

    GenericSharedBuffer faces = getChunk(file, h.facesCount*sizeof(ivec3));

    std::vector<VertexAttribute> attributes;

    switch (h.vertexPacking)
    {
        case COMPRESSED_UVEC4_PACKING :
            {
                GenericSharedBuffer data = getChunk(file, 4*v1size);
                attributes.push_back(VertexAttribute(data, 0, h.verticesCount, 4, GL_UNSIGNED_INT, false));
            }
            break;

        case DEFAULT_PACKING :
        default:
            {
                GenericSharedBuffer positions = getChunk(file, 3*v1size);
                attributes.push_back(VertexAttribute(positions, 0, h.verticesCount, 3, GL_FLOAT, false));
                
                GenericSharedBuffer normals = getChunk(file, 3*v1size);
                attributes.push_back(VertexAttribute(normals, 1, h.verticesCount, 3, GL_FLOAT, false));
                
                GenericSharedBuffer uvs = getChunk(file, 2*v1size);
                attributes.push_back(VertexAttribute(uvs, 2, h.verticesCount, 2, GL_FLOAT, false));
            }
            break;
    }

    bool animated = false;

    if(h.type == VulpineMesh_Type::ELEMENTS_SKINNED)
    {
        animated = true;

        GenericSharedBuffer weights = getChunk(file, 4*v1size);
        attributes.push_back(VertexAttribute(weights, 6, h.verticesCount, 4, GL_FLOAT, false));

        GenericSharedBuffer weightsID = getChunk(file, 4*v1size);
        attributes.push_back(VertexAttribute(weightsID, 5, h.verticesCount, 4, GL_INT, false));
    }

    if(!file.good())
    {
        FILE_ERROR_MESSAGE(filename, "The file is probably corrupted or don't follow vulpineMesh specifications. The loader will return an empty object.")
        return MeshVao();
    }

    MeshVao vao(new VertexAttributeGroup(attributes));

    vao.faces = faces;
    vao.nbFaces = h.facesCount*3;
    vao.animated = animated;

    vao->setAABB(h.AABBmin, h.AABBmax);

    vao->generate();
    vao.generateEBO();

    return vao;
};

void Skeleton::load(const char* filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if(!file)
    {
        FILE_ERROR_MESSAGE(filename, strerror(errno) << "! The loader will return an empty object.");
        return;
    }

    VulpineSkeleton_Header h;
    file.read((char *)&h, sizeof(VulpineSkeleton_Header));

    resize(h.bonesCount);
    file.read((char*)data(), h.bonesCount*sizeof(SkeletonBone));

    if(!file.good())
    {
        FILE_ERROR_MESSAGE(filename, "The file is probably corrupted or don't follow vulpineSkeleton specifications. The loader will return an empty object.")
        return;
    }
}

void Skeleton::initRest()
{
    const size_t s = size();
    for (size_t i = 0; i < s; i++)
    {
        SkeletonBone &b = at(i);
        if(b.parent >= 0)
            b.t = at(b.parent).t * b.t;
    }

    for (size_t i = 0; i < s; i++)
        at(i).t = inverse(at(i).t);
}

