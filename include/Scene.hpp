#ifndef SCENE_HPP
#define SCENE_HPP

#include <Mesh.hpp>
#include <list>
#include <Light.hpp>

typedef std::shared_ptr<Light> SceneLight;
typedef std::shared_ptr<DirectionLight> SceneDirectionalLight;
typedef std::shared_ptr<PointLight> ScenePointLight;
#define newLight std::make_shared<Light>
#define newDirectionLight std::make_shared<DirectionLight>
#define newPointLight std::make_shared<PointLight>

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

        LightBuffer ligthBuffer;
        std::list<SceneLight> lights;

    public :
        Scene();
        void add(std::shared_ptr<Mesh> mesh, bool sort = true);
        void add(SceneLight light);
        void draw();
};


#endif