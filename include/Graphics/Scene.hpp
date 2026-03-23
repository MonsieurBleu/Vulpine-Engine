#ifndef SCENE_HPP
#define SCENE_HPP

#include <Graphics/ObjectGroup.hpp>
#include <Graphics/Camera.hpp>
#include <Timer.hpp>

struct MeshGroup
{
    MeshMaterial material;
    std::deque<ModelRef> meshes;
    std::deque<ModelRef> staticMeshes;

    MeshGroup(MeshMaterial _material)
    {
        material = _material;
    }

    MeshGroup& add(ModelRef mesh, bool isStatic)
    {
        if(isStatic)
            staticMeshes.push_back(mesh);
        else 
            meshes.push_back(mesh);
        return *this;
    }

    uint draw(int layer, bool useBindlessTextures);
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
        std::deque<ObjectGroupRef> staticGroups;

        void addGroupElement(ObjectGroupRef group, bool sort, bool isStatic);
        void removeGroupElement(ObjectGroupRef group);

        uint drawcnt = 0;

        bool useClusteredLighting = false;

        void generateLightClusters();

    public :

        StaticSceneOctree tree;

        Scene();
        Scene& add(ModelRef mesh, bool sort = true, bool isStatic = false);
        Scene& add(SceneLight light);
        Scene& add(ObjectGroupRef group, bool sort = true, bool isStatic = false);

        void updateAllObjects();
        void generateShadowMaps();
        void genLightBuffer();
        uint draw(int layer);
        void depthOnlyDraw(int layer, Camera *camera, bool cull = false, int layers = 0);

        void remove(ModelRef mesh);
        void remove(SceneLight light);
        void remove(ObjectGroupRef group);

        /**
         * @brief Cull all MeshModel3D in the scene
         * @attention No arguments, works with the globals.currentCamera attribute
         */
        void cull(int layer = 0, int cameraLayers = 1);

        void activateTreeCulling(vec3 worldSize);

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
        BenchTimer occlusionCullingTime = BenchTimer("Occlusion CUlling");
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