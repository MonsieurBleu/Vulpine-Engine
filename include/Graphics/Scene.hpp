#ifndef SCENE_HPP
#define SCENE_HPP

#include <Graphics/ObjectGroup.hpp>
#include <Graphics/Camera.hpp>
#include <Timer.hpp>

struct MeshGroup
{
    MeshMaterial material;
    std::deque<ModelRef> meshes;

    MeshGroup(MeshMaterial _material)
    {
        material = _material;
    }

    MeshGroup& add(ModelRef mesh)
    {
        meshes.push_back(mesh);
        return *this;
    }

    uint draw(bool useBindlessTextures);
};

class Scene
{
    private : 
        std::deque<MeshGroup> meshes;
        std::deque<ModelRef> unsortedMeshes;

        LightBuffer lightBuffer;
        ClusteredLightBuffer clusteredLight;

        std::deque<SceneLight> lights;
        std::deque<ObjectGroupRef> groups;

        void addGroupElement(ObjectGroupRef group);
        void removeGroupElement(ObjectGroupRef group);

        uint drawcnt = 0;

        bool useClusteredLighting = false;

        void generateLightClusters();

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

        /**
         * @brief Cull all MeshModel3D in the scene
         * @attention No arguments, works with the globals.currentCamera attribute
         */
        void cull();

        uint getDrawCalls();
        uint getPolyCount();
        uint getVertexCount();
        uint getMaterialCount();
        uint getTotalMeshes();
        uint getShadowMapCount();

        BenchTimer cullTime = BenchTimer("Scene Culling");
        BenchTimer callsTime = BenchTimer("Calls");
        BenchTimer depthOnlyCallsTime = BenchTimer("Depth Only Calls");
        BenchTimer shadowPassCallsTime = BenchTimer("Shadow Pass Calls");
        BenchTimer lightBufferTime = BenchTimer("Light Buffer Creation");
        void endTimers();

        MeshMaterial depthOnlyMaterial;

        bool useBindlessTextures = false;
        bool isUSingClusteredLighting(){return useClusteredLighting;};
        void activateClusteredLighting(ivec3 dimention = ivec3(16, 9, 24), float vFar = 5e3);
        void deactivateClusteredLighting();

        ClusteredLightBuffer& getClusteredLight(){return clusteredLight;};

        const std::deque<SceneLight>& getLights(){return lights;};
};


#endif