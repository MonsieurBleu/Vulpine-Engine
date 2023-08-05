#ifndef SCENE_HPP
#define SCENE_HPP

#include <Mesh.hpp>
#include <list>

struct MeshGroup
{
    MeshMaterial material;
    std::list<std::shared_ptr<Mesh>> meshes;

    MeshGroup(MeshMaterial _material)
    {
        material = _material;
    }

    MeshGroup& add(std::shared_ptr<Mesh> mesh)
    {
        meshes.push_back(mesh);
        return *this;
    }

    void draw();
};

class Scene
{
    private : 
        std::list<MeshGroup> meshes;
        std::list<std::shared_ptr<Mesh>> unsortedMeshes;

    public :
        
        void add(std::shared_ptr<Mesh> mesh, bool sort = true);
        void draw();
};


#endif