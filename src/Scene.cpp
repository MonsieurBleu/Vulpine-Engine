#include <Scene.hpp>
#include <iostream>
#include <Globals.hpp> 
#include <Helpers.hpp>

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

Scene& Scene::add(ModelRef mesh, bool sort)
{
    if(!sort)
    {
        unsortedMeshes.push_back(mesh);
        return *this;
    }

    for(auto i = meshes.begin(); i != meshes.end(); i++)
    {
        if(i->material.get() == mesh->getMaterial().get())
        {
            i->add(mesh);
            return *this;
        }
    }

    meshes.push_back(MeshGroup(mesh->getMaterial()).add(mesh));
    return *this;
};

Scene& Scene::add(SceneLight light)
{
    lights.push_back(light);
    return *this;
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

// TODO : test
void Scene::removeGroupElement(ObjectGroupRef group)
{
    for(auto i = group->meshes.begin(); i != group->meshes.end(); i++)
    {
        remove(*i);
    }

    for(auto i = group->lights.begin(); i != group->lights.end(); i++)
    {
        remove(*i);
    }

    for(auto i = group->children.begin(); i != group->children.end(); i++)
    {
        removeGroupElement(*i);
    }
}

Scene& Scene::add(ObjectGroupRef group)
{
    addGroupElement(group);

    groups.push_back(group);

    return *this;
}

uint MeshGroup::draw()
{
    material->activate();
    
    uint drawcnt = 0;

    for(auto i : meshes)
    if(i->isCulled())
    {
        i->bindAllMaps();
        drawcnt += i->drawVAO();
    }

    material->deactivate();
    return drawcnt;
}

void Scene::updateAllObjects()
{
    for(auto i = groups.begin(); i != groups.end(); i++)
        (*i)->update();
}

void Scene::genLightBuffer()
{
    for(auto i = lights.begin(); i != lights.end(); i++)
    {
        ligthBuffer.add(**i);
        (*i)->bindShadowMap();
    }
    
    ligthBuffer.update();
    ligthBuffer.activate(0);
}

uint Scene::draw()
{
    drawcnt = 0;

    for(auto i = meshes.begin(); i != meshes.end(); i++)
        drawcnt += i->draw();
    
    for(auto i : unsortedMeshes)
        if(i->isCulled())
            drawcnt += i->draw();

    ligthBuffer.reset();
    return drawcnt;
}

uint Scene::getDrawCalls(){return drawcnt;}

// TODO : Add a way to have custom depth vertex shader to some object
// NOTE : If an object have a non trivial vertex shader, maybe the vertices
//        should be calculated just once in a compute/cpu prepass. But this 
//        woulnd't be compatible with tesselation.

void Scene::depthOnlyDraw(Camera &camera, bool cull)
{
    if(depthOnlyMaterial != NULL)
    {
        for(auto i = meshes.begin(); i != meshes.end(); i++)
        {
            if(i->material.depthOnly)
                i->material.depthOnly->activate();
            else
                depthOnlyMaterial->activate();
            
            ShaderProgram *dom = i->material.depthOnly ? i->material.depthOnly.get() : depthOnlyMaterial.get();
            dom->activate();

            ShaderUniform(camera.getProjectionViewMatrixAddr(), 2).activate();
            ShaderUniform(camera.getViewMatrixAddr(), 3).activate();
            ShaderUniform(camera.getProjectionMatrixAddr(), 4).activate();
            ShaderUniform(camera.getPositionAddr(), 5).activate();
            ShaderUniform(camera.getDirectionAddr(), 6).activate();

            if(cull)
            {
                for(auto j : i->meshes)
                    if(j->isCulled())
                    {
                        j->bindAllMaps();
                        j->drawVAO();
                    }
            }
            else
                for(auto j : i->meshes)
                {
                    j->bindAllMaps();
                    j->drawVAO();
                }

            dom->deactivate();
        }

        depthOnlyMaterial->deactivate();
    }
    
    /*
    We consider that unsorted meshes have special materials, like transparents ones,
    that cannot be used on depthOnlyDraw. So for the moment, this is disabled.

    if(cull)
        for(auto i : unsortedMeshes)
            if(i->cull())
                i->drawVAO();
    else
        for(auto i : unsortedMeshes)
            i->drawVAO();
    */
}

void Scene::generateShadowMaps()
{
    Camera *tmp = globals.currentCamera;

    for(auto i : lights)
        if(i->getInfos()._infos.b&LIGHT_SHADOW_ACTIVATED)
        {
            i->shadowMap.activate();
            i->updateShadowCamera();
            i->shadowCamera.updateFrustum();
            globals.currentCamera = &i->shadowCamera;
            //cull();
            depthOnlyDraw(i->shadowCamera);
            i->shadowMap.deactivate();
        }
    
    globals.currentCamera = tmp;
}

void Scene::remove(ModelRef mesh)
{
    for(auto i = meshes.begin(); i != meshes.end(); i++)
    {
        if(i->material.get() == mesh->getMaterial().get())
            for(auto j = i->meshes.begin(); j != i->meshes.end(); j++)
                if(j->get() == mesh.get())
                {
                    i->meshes.erase(j);
                    return;
                }
    }

    for(auto i = unsortedMeshes.begin(); i != unsortedMeshes.end(); i++)
    {
        if(i->get() == mesh.get())
        {
            unsortedMeshes.erase(i);
            return;
        }
    }
}

void Scene::remove(SceneLight light)
{
    for(auto i = lights.begin(); i != lights.end(); i++)
        if(i->get() == light.get())
        {
            lights.erase(i);
            return;
        }
}

void Scene::remove(ObjectGroupRef group)
{
    removeGroupElement(group);

    for(auto i = groups.begin(); i != groups.end(); i++)
        if(i->get() == group.get())
        {
            groups.erase(i);
            return;
        }
}

void Scene::cull()
{
    for(auto i = meshes.begin(); i != meshes.end(); i++)
    {
        for(auto j : i->meshes)
            j->cull();
    }

    for(auto i : unsortedMeshes)
        i->cull();
}
