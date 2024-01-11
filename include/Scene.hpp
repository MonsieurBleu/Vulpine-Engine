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

    uint draw();
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

        uint drawcnt = 0;

    public :
        Scene();
        Scene& add(ModelRef mesh, bool sort = true);
        Scene& add(SceneLight light);
        Scene& add(ObjectGroupRef group);

        void updateAllObjects();
        void generateShadowMaps();
        void genLightBuffer();
        uint draw();
        void depthOnlyDraw(Camera &camera, bool cull = false);

        void remove(ModelRef mesh);
        void remove(SceneLight light);
        void remove(ObjectGroupRef group);

        uint getDrawCalls();

        MeshMaterial depthOnlyMaterial;
};


#endif