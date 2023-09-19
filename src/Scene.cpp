#include <Scene.hpp>
#include <iostream>

Scene::Scene()
{
    // ligthBuffer
    // .add(
    //     PointLight()
    //         .setColor(vec3(1, 0, 0))
    //         .setPosition(vec3(100, 50, 0))
    //         .setIntensity(1)
    //         .setSize(vec3(2, 1, 1)))
    // .add(
    //     DirectionLight()
    //         .setColor(vec3(0, 1, 0))
    //         .setDirection(vec3(0, 1, 0))
    //         .setIntensity(1))
    // .add(
    //     DirectionLight()
    //         .setColor(vec3(0, 0, 1))
    //         .setDirection(vec3(1, 0, 0))
    //         .setIntensity(2))
    // ;

    // lights
    //     .push_back(
    //         std::make_shared<Light>(
    //             PointLight()
    //                 .setColor(vec3(1, 0, 0))
    //                 .setPosition(vec3(100, 50, 0))
    //                 .setIntensity(1)
    //                 .setSize(vec3(2, 1, 1))));


    ligthBuffer.send();

}

void Scene::add(ModelRef mesh, bool sort)
{
    if(!sort)
    {
        unsortedMeshes.push_back(mesh);
        return;
    }

    for(auto i = meshes.begin(); i != meshes.end(); i++)
    {
        if(i->material.get() == mesh->getMaterial().get())
        {
            i->add(mesh);
            return;
        }
    }

    meshes.push_back(MeshGroup(mesh->getMaterial()).add(mesh));
};

void Scene::add(SceneLight light)
{
    lights.push_back(light);
}

void Scene::addGroupElement(ObjectGroupRef group)
{
    for(auto i = group->meshes.begin(); i != group->meshes.end(); i++)
    {
        add(*i);
    }

    for(auto i = group->lights.begin(); i != group->lights.end(); i++)
    {
        add(*i);
    }

    for(auto i = group->children.begin(); i != group->children.end(); i++)
    {
        addGroupElement(*i);
    }
}

void Scene::add(ObjectGroupRef group)
{
    addGroupElement(group);

    groups.push_back(group);
}

void MeshGroup::draw()
{
    material->activate();

    for(auto i = meshes.begin(); i != meshes.end(); i++)
        (*i)->drawVAO();

    material->deactivate();
}

void Scene::draw()
{
    for(auto i = groups.begin(); i != groups.end(); i++)
        (*i)->update();

    for(auto i = lights.begin(); i != lights.end(); i++)
        ligthBuffer.add(**i);

    ligthBuffer.update();
    ligthBuffer.activate(0);

    for(auto i = meshes.begin(); i != meshes.end(); i++)
    {
        i->draw();
    }
    
    for(auto i = unsortedMeshes.begin(); i != unsortedMeshes.end(); i++)
    {
        (*i)->draw();
    }

    ligthBuffer.reset();
}