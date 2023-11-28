#ifndef SCENE_HPP
#define SCENE_HPP

#include <ObjectGroup.hpp>
#include <Camera.hpp>

struct MeshGroup
{
    MeshMaterial material;
    std::list<ModelRef> meshes;

    MeshGroup(MeshMaterial _material)
    {
        material = _material;
    }

    MeshGroup& add(ModelRef mesh)
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
        std::list<ModelRef> unsortedMeshes;

        LightBuffer ligthBuffer;
        std::list<SceneLight> lights;
        std::list<ObjectGroupRef> groups;

        void addGroupElement(ObjectGroupRef group);
        void removeGroupElement(ObjectGroupRef group);

        void depthOnlyDraw(Camera &camera);

    public :
        Scene();
        void add(ModelRef mesh, bool sort = true);
        void add(SceneLight light);
        void add(ObjectGroupRef group);

        void updateAllObjects();
        void generateShadowMaps();
        void genLightBuffer();
        void draw();

        void remove(ModelRef mesh);
        void remove(SceneLight light);
        void remove(ObjectGroupRef group);

        MeshMaterial depthOnlyMaterial;
};


#endif