#ifndef OBJECT_GROUP_HPP
#define OBJECT_GROUP_HPP

#include <deque>

#include <Mesh.hpp>
#include <Light.hpp>

#define ObjectGroupRef std::shared_ptr<ObjectGroup>
#define newObjectGroup std::make_shared<ObjectGroup>

class Scene;

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

    void remove(ModelRef mesh);

    ObjectGroupRef copy();
};

#endif