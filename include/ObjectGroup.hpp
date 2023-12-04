#ifndef OBJECT_GROUP_HPP
#define OBJECT_GROUP_HPP

#include <list>

#include <Mesh.hpp>
#include <Light.hpp>

#define ObjectGroupRef std::shared_ptr<ObjectGroup> 
#define newObjectGroup std::make_shared<ObjectGroup>

class Scene;

class ObjectGroup
{
    friend Scene;

    protected :
        ObjectGroup* parent = NULL;

        std::list<ModelStateRef> states;
        std::list<ObjectGroupRef> children;
        std::list<ModelRef> meshes;
        std::list<SceneLight> lights;

    public : 
        ModelState3D state;

        virtual void update(bool forceUpdate = false);
        void add(ModelRef meshe);
        void add(SceneLight light);
        void add(ObjectGroupRef group);
        void add(ModelStateRef state);

};


#endif