#ifndef OBJECT_GROUP_HPP
#define OBJECT_GROUP_HPP

#include <deque>

#include <Graphics/Mesh.hpp>
#include <Graphics/Light.hpp>

#define ObjectGroupRef std::shared_ptr<ObjectGroup>
#define newObjectGroup std::make_shared<ObjectGroup>

class Scene;

class FastUI_valueMenu;

#include <iostream>

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

protected:
    ObjectGroup *parent = NULL; // unused

    std::deque<ModelStateRef> states;
    std::deque<ObjectGroupRef> children;
    std::deque<ModelRef> meshes;
    std::deque<SceneLight> lights;


    void populateMeshesToBeMerged(std::vector<meshesToBeMerged> &mtbm);
    void removeEmptyChildren();


public:
    std::string name;

    ModelState3D state;
    
    virtual void update(bool forceUpdate = false);
    void add(ModelRef meshe);
    void add(SceneLight light);
    void add(ObjectGroupRef group);
    void add(ModelStateRef state);

    void setAnimation(SkeletonAnimationState *animation);
    void setMenu(FastUI_valueMenu &menu, std::u32string name);

    void remove(ModelRef mesh);
    void remove(ObjectGroupRef group);

    std::pair<vec3, vec3> getMeshesBoundingBox();

    ObjectGroupRef copy();

    std::deque<ModelRef>& getMeshes() {return meshes;};
    std::deque<ObjectGroupRef>& getChildren(){return children;};
    std::deque<ModelStateRef>& getStates(){return states;};
    std::deque<SceneLight>& getLights(){return lights;};

    /*
        Return a dupplicate of the group, 
        with batched together compatible meshes that uses vertex packing.

        Note : Animated, Batched or Instanced meshes can't be merged together.
    */
    ObjectGroupRef optimizedBatchedCopy();
};

#endif