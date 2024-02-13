#include <Scene.hpp>
#include <iostream>
#include <Globals.hpp> 
#include <Helpers.hpp>
#include <MathsUtils.hpp>

Scene::Scene()
{
    // lightBuffer
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


    lightBuffer.send();

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

uint MeshGroup::draw(bool useBindlessTextures)
{
    material->activate();
    
    uint drawcnt = 0;
    
    if(useBindlessTextures)
    {
        for(auto i : meshes)
            if(i->isCulled())
            {
                i->setBindlessMaps();
                drawcnt += i->drawVAO();
            }
    }
    else
    {
        for(auto i : meshes)
            if(i->isCulled())
            {
                i->bindAllMaps();
                drawcnt += i->drawVAO();
            }
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
    for(auto i : lights)
    {
        lightBuffer.add(*i);
        i->bindShadowMap();
    }
    
    lightBuffer.update();
    lightBuffer.activate(0);

    if(useClusteredLighting)
    {
        generateLightClusters();
        clusteredLight.update();
        clusteredLight.activate(1);
    }
}

void Scene::generateLightClusters()
{
    const float vFar = 5e3;

    const ivec3 dim = clusteredLight.dim();
    const vec3 dimf = vec3(dim);
    const vec3 idim = 2.f/vec3(dim);
    int* buff = clusteredLight.get();
    int clusterOff = 0;

    LightInfos* lbuff = lightBuffer.get();
    const int lmax = lightBuffer.maxID();

    const mat4 vm = globals.currentCamera->getViewMatrix();
    const mat4 im = inverse(globals.currentCamera->getProjectionMatrix());

    const mat pm = globals.currentCamera->getProjectionMatrix();

    /* Transforming all lights to view coordinates */
    LightInfos* lbuffv = new LightInfos[lmax];
    memcpy(lbuffv, lbuff, lmax*sizeof(LightInfos));    
    for(int i = 0; i < lmax; i++)
    {
        LightInfos &l = lbuffv[i];
        switch (l._infos.a)
        {
            case POINT_LIGHT :
            {
                float tmp  = l._position.a;
                l._position.a = 1.f;
                l._position = vm * l._position;
                l._position.a = tmp;
            }
                break;

            default: break;
        }
    }

    std::pair<vec3, vec3> AABBs[clusteredLight.gridSize()];
    int aabbID = 0;
    for(float x = 1.f; x <= dimf.x; x++)
    for(float y = 1.f; y <= dimf.y; y++)
    for(float z = 1.f; z <= dimf.z; z++, aabbID++)
    {
        float viewX2 = x*idim.x - 1.f; 
        float viewX = viewX2 - idim.x; 

        float inverseZ = (viewX2 > 1e-6);
        float viewZid = !inverseZ ? z : max(z-1.f, 1e-6f);
        float viewZ2id = inverseZ ? z : max(z-1.f, 1e-6f);
        float viewZ = dimf.z/(viewZid*vFar);
        float viewZ2 = dimf.z/(viewZ2id*vFar);

        float viewYbase = y*idim.y - 1.f;
        bool inverseY = inverseZ != (viewYbase >= 0.f);
        float viewY = viewYbase - (!inverseY ? idim.y : 0.f);
        float viewY2 = viewYbase - (inverseY ? idim.y : 0.f);

        vec3 p1 = viewToWorld(vec4(viewX, viewY, viewZ, 1), im);
        vec3 p2 = viewToWorld(vec4(viewX2, viewY2, viewZ2, 1), im);

        AABBs[aabbID].first = min(p1, p2);
        AABBs[aabbID].second = max(p1, p2);
    }

    /****** Per Cluster Culling 
    aabbID = 0;
    for(float x = 1.f; x <= dimf.x; x++)
    for(float y = 1.f; y <= dimf.y; y++)
    for(float z = 1.f; z <= dimf.z; z++, aabbID++, clusterOff += MAX_LIGHT_PER_CLUSTER)
    {
        vec3 minP = AABBs[aabbID].first;
        vec3 maxP = AABBs[aabbID].second;

        int id = clusterOff;
        int maxId = clusterOff+MAX_LIGHT_PER_CLUSTER;
        for(int i = 0; i < lmax && id < maxId; i++)
        {
            const LightInfos &l = lbuffv[i];
            bool culled = false;

            switch (l._infos.a)
            {
                case  POINT_LIGHT :
                {
                    const vec3 p = vec3(l._position);
                    const vec3 closest = p-max(minP, min(maxP, p));
                    culled = dot(closest, closest) <= l._direction.x*l._direction.x;
                }
                    break;

                default: break;
            }

            if(culled) buff[id++] = i;
        }

        buff[id] = lmax;
    }
    ******/

    /****** Per Light Optimized Culling ******/
    int buffSize = MAX_LIGHT_PER_CLUSTER*dim.x*dim.y*dim.z;
    for(int i = 0; i < buffSize; i+= MAX_LIGHT_PER_CLUSTER)
        buff[i] = lmax;

    for(int i = 0; i < lmax; i++)
    {
        const LightInfos &l = lbuffv[i];

        if(l._infos.a != POINT_LIGHT) continue;

        vec4 zSp = vec4(0, 0, l._position.z + l._direction.x, 1) * pm;
        float zId = 100.f*(zSp.w/zSp.z)*dimf.z/vFar;
        int minZ = clamp((int)(zId), 0, dim.z);

        vec4 zSp2 = vec4(0, 0, l._position.z - l._direction.x, 1) * pm;
        float zId2 = 100.f*(zSp2.w/zSp2.z)*dimf.z/vFar;
        int maxZ = clamp((int)(zId2)+1, 0, dim.z);
        
        if(!maxZ) continue;

        // vec4 xSp = vec4(l._position.x - l._direction.x, l._position.y, l._position.z - l._direction.x, 1) * pm;
        // float xId = (0.1f*(xSp.x/xSp.w) + 1.f)*0.5*dimf.x;
        // int minX = dim.x-clamp((int)(xId)+2, 0, dim.x);

        // vec4 xSp2 = vec4(l._position.x + l._direction.x, l._position.y, l._position.z - l._direction.x, 1) * pm;
        // float xId2 = (0.1f*(xSp2.x/xSp2.w) + 1.f)*0.5*dimf.x;
        // int maxX = dim.x-clamp((int)(xId2)-2, 0, dim.x);
        // std::cout << minX << "\n";

        bool lastOneCulledX = false;
        int culledSwitchX = 0;
        for(int x = 0; x < dim.x; x++)
        {
            bool oneCulledX = false;

            for(int y = 0; y < dim.y; y++)
            for(int z = minZ; z < maxZ; z++)
            {
                aabbID = x*dim.z*dim.y + y*dim.z + z;
                vec3 minP = AABBs[aabbID].first;
                vec3 maxP = AABBs[aabbID].second;

                const vec3 p = vec3(l._position);
                const vec3 closest = p-max(minP, min(maxP, p));
                bool culled = dot(closest, closest) <= l._direction.x*l._direction.x;

                if(culled)
                {
                    oneCulledX = true;
                    int cid = MAX_LIGHT_PER_CLUSTER*aabbID;
                    const int maxCID = cid + MAX_LIGHT_PER_CLUSTER;
                    while(buff[cid] != lmax && cid < maxCID) cid++;
                    
                    buff[cid] = i;
                    buff[cid+1] = lmax;
                }
            }

            culledSwitchX += lastOneCulledX != oneCulledX ? 1 : 0;
            if(culledSwitchX >= 2)
                break;

            lastOneCulledX = oneCulledX;
        }
    }

    delete lbuffv;
}

uint Scene::draw()
{
    drawcnt = 0;

    for(auto i = meshes.begin(); i != meshes.end(); i++)
        drawcnt += i->draw(useBindlessTextures);
    
    for(auto i : unsortedMeshes)
        if(i->isCulled())
        {
            if(useBindlessTextures)
                i->setBindlessMaps();
            else
                i->bindAllMaps();
            
            i->getMaterial()->activate();
            drawcnt += i->drawVAO();
            i->getMaterial()->deactivate();
        }

    lightBuffer.reset();
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

            // if(cull)
            // {
            //     for(auto j : i->meshes)
            //         if(j->isCulled())
            //         {
            //             j->bindAllMaps();
            //             j->drawVAO();
            //         }
            // }
            // else
            //     for(auto j : i->meshes)
            //     {
            //         j->bindAllMaps();
            //         j->drawVAO();
            //     }

            if(useBindlessTextures)
            {
                for(auto j : i->meshes)
                    {
                        j->setBindlessMaps();
                        j->drawVAO(true);
                    }
            }
            else
            {
                for(auto j : i->meshes)
                {
                    j->bindAllMaps();
                    j->drawVAO(true);
                }
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

void Scene::activateClusteredLighting(ivec3 dimention)
{
    useClusteredLighting = true;
    clusteredLight.allocate(dimention);
    clusteredLight.send();
}

void Scene::deactivateClusteredLighting()
{
    useClusteredLighting = false;
}