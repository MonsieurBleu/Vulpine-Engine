#include <Scene.hpp>
#include <iostream>

void Scene::add(std::shared_ptr<Mesh> mesh, bool sort)
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

void MeshGroup::draw()
{
    material->activate();

    for(auto i = meshes.begin(); i != meshes.end(); i++)
        (*i)->drawVAO();

    material->deactivate();
}

void Scene::draw()
{
    for(auto i = meshes.begin(); i != meshes.end(); i++)
        i->draw();
    
    for(auto i = unsortedMeshes.begin(); i != unsortedMeshes.end(); i++)
        (*i)->draw();
}