#ifndef OBJECT_GROUP_HPP
#define OBJECT_GROUP_HPP

#include <deque>

#include <Graphics/Mesh.hpp>
#include <Graphics/Light.hpp>

#include <functional>

#include <Graphics/Query.hpp>

#define ObjectGroupRef std::shared_ptr<ObjectGroup>
#define newObjectGroup std::make_shared<ObjectGroup>

class Scene;

class FastUI_valueMenu;

// #include <iostream>

#define QUADTREE_ITERATION 5

struct StaticSceneOctree
{
    struct ID{
        int8 coord[QUADTREE_ITERATION] = {-1};
    };

    struct Node
    {
        enum CullingStatus : uint8
        {
            NotVisible,
            Undefined,
            FullyVisible,
            PartiallyVisible

        } status[4];

        bool doOcclusionQuerry[4] = {false};

        bool drawElements[4] = {false};

        OcclussionQuery *query = nullptr;

        int recursiveElementCounter = 0;
        int nodeDepth = 0;

        vec3 middle;
        vec3 size;

        std::deque<ObjectGroupRef> elements;

        std::shared_ptr<Node> childs[8];

        uint cull(int layer, int cameraLayers = 1);

        void updateOcclusion();

        bool add(ObjectGroupRef group, ID id, int depth);

        bool remove(ObjectGroupRef group, ID id, int depth);

        int nodesCount();

        void recusriveCall(std::function<void(Node &node)> f);


        void propagateFullyVisible(int layer);
        void propagateNotVisible(int layer);


        std::string toStr();
    };


    inline static vec3 worldSize = vec3(4096, 1024, 4096);
    inline static vec3 worldCenter = vec3(0, 256, 0);
    inline static float bonusSpace = 64.f/4096;

    Node root;

    ID getID(vec3 min, vec3 max);

    ID add(ObjectGroupRef group);

    bool remove(ObjectGroupRef group);

    std::string toStr();
};


class ObjectGroup
{

    struct meshesToBeMerged
    {
        meshesToBeMerged(std::pair<ObjectGroup*, ModelRef> m);

        MeshMaterial material;
        ShaderUniformGroup baseUniforms;
        ShaderUniformGroup uniforms;
        std::vector<Texture2D> maps;
        std::vector<int> mapsLocation;

        std::vector<std::pair<ObjectGroup*, ModelRef>> models;

        bool isCompatible(ModelRef m);
    };



    friend Scene;
    friend StaticSceneOctree;

protected:
    ObjectGroup *parent = NULL; // unused

    std::deque<ModelInstanceRef> instances;
    std::deque<ModelStateRef> states;
    std::deque<ObjectGroupRef> children;
    std::deque<ModelRef> meshes;
    std::deque<SceneLight> lights;


    void populateMeshesToBeMerged(std::vector<meshesToBeMerged> &mtbm);
    void removeEmptyChildren();

    bool useStaticAABB = false;
    std::pair<vec3, vec3> meshesBoundingBox;

    StaticSceneOctree::ID staticSceneID;

    bool externCulledValue = false;
    bool externCull = false;

public:
    std::string name;

    ObjectGroup(){state.externCullPtr = &externCulledValue;};
    ObjectGroup(std::string name) : name(name){state.externCullPtr = &externCulledValue;};
    ObjectGroup(std::string name, bool *externCullPtr) : name(name){state.externCullPtr = &externCulledValue; if(externCullPtr) state.externCullPtr = externCullPtr;};

    ModelState3D state;
    
    virtual void update(bool forceUpdate = false);
    void propagateHideStatus();
    void add(ModelRef meshe);
    void add(SceneLight light);
    void add(ObjectGroupRef group);
    void add(ModelStateRef state);
    void add(ModelInstanceRef instance);

    void setAnimation(SkeletonAnimationState *animation);
    void setMenu(FastUI_valueMenu &menu, std::u32string name);

    void setStaticAABB(vec3 min, vec3 max);

    void remove(ModelRef mesh);
    void remove(ObjectGroupRef group);

    void propagateExternCullMode(int layer, bool culled);
    void propagateExternCullPtr(bool *externCullPtr);
    void disableExternCullMode();
    bool isCulled();

    void updateMeshesBoundingBox();
    std::pair<vec3, vec3> getMeshesBoundingBox();

    ObjectGroupRef copy(bool *externCullPtr = nullptr);

    std::deque<ModelRef>& getMeshes() {return meshes;};
    std::deque<ObjectGroupRef>& getChildren(){return children;};
    std::deque<ModelStateRef>& getStates(){return states;};
    std::deque<SceneLight>& getLights(){return lights;};
    std::deque<ModelInstanceRef>& getInstances(){return instances;};

    void iterateOnAllMesh_Recursive(std::function<void(ModelRef)> f);

    /*
        Return a dupplicate of the group, 
        with batched together compatible meshes that uses vertex packing.

        Note : Animated, Batched or Instanced meshes can't be merged together.
    */
    ObjectGroupRef optimizedBatchedCopy();
};

#endif