#ifndef OBJECT_GROUP_HPP
#define OBJECT_GROUP_HPP

#include <deque>

#include <Mesh.hpp>
#include <Light.hpp>

#define ObjectGroupRef std::shared_ptr<ObjectGroup>
#define newObjectGroup std::make_shared<ObjectGroup>

class Scene;

class FastUI_valueMenu;

#include <iostream>

class ObjectGroup
{
    friend Scene;

protected:
    ObjectGroup *parent = NULL; // unused

    std::deque<ModelStateRef> states;
    std::deque<ObjectGroupRef> children;
    std::deque<ModelRef> meshes;
    std::deque<SceneLight> lights;

public:
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

    const std::deque<ModelRef>& getMeshes() const {return meshes;};
    const std::deque<ObjectGroupRef>& getChildren() const {return children;};
    const std::deque<ModelStateRef>& getStates() const {return states;};
    const std::deque<SceneLight>& getLights() const {return lights;};
};

#endif