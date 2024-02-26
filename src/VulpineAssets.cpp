#include <VulpineAssets.hpp>
#include <Skeleton.hpp>

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

MeshVao loadVulpineMesh(const std::string &filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if(!file)
    {
        std::cerr
        << TERMINAL_ERROR << "Error loading file : "
        << TERMINAL_FILENAME << filename
        << TERMINAL_ERROR << "\n";
        perror("\treadOBJ");

        std::cerr << "\tThe loader will return an empty object.\n"<< TERMINAL_RESET;

        return MeshVao();
    }

    VulpineMesh_Header h;

    file.read((char *)&h, sizeof(VulpineMesh_Header));

    const float v1size = h.verticesCount*sizeof(float);

    GenericSharedBuffer faces = getChunk(file, h.facesCount*sizeof(ivec3));

    std::vector<VertexAttribute> attributes;

    GenericSharedBuffer positions = getChunk(file, 3*v1size);
    attributes.push_back(VertexAttribute(positions, 0, h.verticesCount, 3, GL_FLOAT, false));
    
    GenericSharedBuffer normals = getChunk(file, 3*v1size);
    attributes.push_back(VertexAttribute(normals, 1, h.verticesCount, 3, GL_FLOAT, false));
    
    GenericSharedBuffer uvs = getChunk(file, 2*v1size);
    attributes.push_back(VertexAttribute(uvs, 2, h.verticesCount, 2, GL_FLOAT, false));

    if(h.type == VulpineMesh_Type::ELEMENTS_SKINNED)
    {
        GenericSharedBuffer weights = getChunk(file, 4*v1size);
        attributes.push_back(VertexAttribute(weights, 6, h.verticesCount, 4, GL_FLOAT, false));

        GenericSharedBuffer weightsID = getChunk(file, 4*v1size);
        attributes.push_back(VertexAttribute(weightsID, 5, h.verticesCount, 4, GL_INT, false));
    }

    if(!file.good())
    {
        std::cerr
        << TERMINAL_ERROR << "Error reading vulpineMesh file : "
        << TERMINAL_FILENAME << filename
        << TERMINAL_ERROR << "\n\tThe loader will return an empty object.\n"
        << TERMINAL_RESET;

        return MeshVao();
    }

    MeshVao vao(new VertexAttributeGroup(attributes));

    vao.faces = faces;
    vao.nbFaces = h.facesCount*3;

    vao->setAABB(h.AABBmin, h.AABBmax);

    return vao;
};

void Skeleton::load(const std::string &filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if(!file)
    {
        std::cerr
        << TERMINAL_ERROR << "Error loading file : "
        << TERMINAL_FILENAME << filename
        << TERMINAL_ERROR << "\n";
        perror("\tSkeleton::load");

        std::cerr << "\tThe loader will return an empty object.\n"<< TERMINAL_RESET;

        return;
    }

    VulpineSkeleton_Header h;
    file.read((char *)&h, sizeof(VulpineSkeleton_Header));

    resize(h.bonesCount);
    file.read((char*)data(), h.bonesCount*sizeof(SkeletonBone));

    if(!file.good())
    {
        std::cerr
        << TERMINAL_ERROR << "Error reading vulpineSkeleton file : "
        << TERMINAL_FILENAME << filename
        << TERMINAL_ERROR << "\n\tThe loader will return an empty object.\n"
        << TERMINAL_RESET;
        return;
    }

    tmpInv.resize(h.bonesCount);

    for(size_t i = 0; i < h.bonesCount; i++)
    {
        SkeletonBone &b = at(i);

        tmpInv[i] = b.t;

        if(b.parent >= 0)
            tmpInv[i] = tmpInv[b.parent] * b.t;
    }

    for(size_t i = 0; i < h.bonesCount; i++)
        tmpInv[i] = inverse(tmpInv[i]);
}



