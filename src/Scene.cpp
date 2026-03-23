#include <Graphics/Scene.hpp>
#include <iostream>
#include <Globals.hpp> 
#include <Helpers.hpp>
#include <MathsUtils.hpp>
#include <App.hpp>


#define PER_LIGHT_OPTIMIZED_CLUSTERED_RENDERING

Scene::Scene()
{
    lightBuffer.send();
}

Scene& Scene::add(ModelRef mesh, bool sort, bool isStatic)
{
    if(!sort || !mesh->sorted)
    {
        mesh->sorted = false;
        
        unsortedMeshes.push_back(mesh);
        return *this;
    }

    // if(putInFront)
    // {
    //     meshes.push_front(MeshGroup(mesh->getMaterial()).add(mesh));
    //     return *this;
    // }

    for(auto i = meshes.begin(); i != meshes.end(); i++)
    {
        if(i->material.get() == mesh->getMaterial().get())
        {
            i->add(mesh, isStatic);
            return *this;
        }
    }

    meshes.push_back(MeshGroup(mesh->getMaterial()).add(mesh, isStatic));
    return *this;
};

Scene& Scene::add(SceneLight light)
{
    lights.push_back(light);
    return *this;
}

void Scene::addGroupElement(ObjectGroupRef group, bool sort, bool isStatic)
{
    for(auto i = group->meshes.begin(); i != group->meshes.end(); i++)
    {
        add(*i, sort, isStatic);
    }

    for(auto i = group->lights.begin(); i != group->lights.end(); i++)
    {
        add(*i);
    }

    for(auto i = group->children.begin(); i != group->children.end(); i++)
    {
        addGroupElement(*i, sort, isStatic);
    }

    // if(group->getInstances().size())
    //     ERROR_MESSAGE("Adding")

    // for(auto &i : group->getInstances())
    // {
    //     if(i and !i->enable) i = i->originalModel->createInstance();
    // }
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

    // if(group->getInstances().size())
    //     ERROR_MESSAGE("Removing")
    for(auto &i : group->getInstances())
    {
        // if(i) i->enable = false;
        if(i.instance && i.originalModel)
        {
            i.originalModel->removeInstance(i.instance);
            i.instance = nullptr;
        } 
    }

    for(auto i = group->children.begin(); i != group->children.end(); i++)
    {
        removeGroupElement(*i);
    }
}

Scene& Scene::add(ObjectGroupRef group, bool sort, bool isStatic)
{
    addGroupElement(group, sort, isStatic);

    if(isStatic)
    {
        staticGroups.push_back(group);

        // auto box = group->getMeshesBoundingBox();

        // tree.getID(box.first, box.second);

        group->update(true);
        tree.add(group);
    }
    else
        groups.push_back(group);


    return *this;
}

uint MeshGroup::draw(int layer, bool useBindlessTextures)
{
    material->activate();
    
    uint drawcnt = 0;
    
    if(useBindlessTextures)
    {
        for(auto &i : staticMeshes)
            if(i->isCulled(layer))
            {
                i->setBindlessMaps();
                drawcnt += i->drawVAO(layer);
            }

        for(auto &i : meshes)
            if(i->isCulled(layer))
            {
                i->setBindlessMaps();
                drawcnt += i->drawVAO(layer);
            }
    }
    else
    {
        for(auto &i : staticMeshes)
            if(i->isCulled(layer))
            {
                i->bindAllMaps();
                drawcnt += i->drawVAO(layer);
            }

        for(auto &i : meshes)
            if(i->isCulled(layer))
            {
                i->bindAllMaps();
                drawcnt += i->drawVAO(layer);
            }

    }

    material->deactivate();
    return drawcnt;
}

void Scene::updateAllObjects()
{
    drawcnt = 0;
    
    for(auto &i : groups)
        i->update();

    // system("clear");
    // NOTIF_MESSAGE(tree.toStr());
}

void Scene::genLightBuffer()
{
    lightBufferTime.start();

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

    lightBufferTime.hold();
}

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

void Scene::generateLightClusters()
{
    const float hNFratio = clusteredLight.ivFar;
    const float near_ = globals.currentCamera->getState().nearPlane;
    const float far_ = clusteredLight.vFar;

    const ivec3 dim = clusteredLight.dim();
    const vec3 dimf = vec3(dim);
    const vec3 idim = 2.f/vec3(dim);
    int* buff = clusteredLight.get();

    LightInfos* lbuff = lightBuffer.get();
    const int lmax = lightBuffer.maxID()-1;

    const mat4 vm = globals.currentCamera->getViewMatrix();
    const mat4 ipm = inverse(globals.currentCamera->getProjectionMatrix());

    const mat4 pm = globals.currentCamera->getProjectionMatrix();

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

    static std::vector<std::pair<vec3, vec3>> AABBs;
    int aabbID = 0;

    bool cameraIsORtho = globals.currentCamera->getType() == CameraType::ORTHOGRAPHIC;

    if(
        // AABBs.empty() || 
        // cameraIsORtho
        true
    )
    {
        AABBs.resize(clusteredLight.gridSize());
        for(float x = 1.f; x <= dimf.x; x++)
        for(float y = 1.f; y <= dimf.y; y++)
        for(float z = 1.f; z <= dimf.z; z++, aabbID++)
        {
            float viewX2 = x*idim.x - 1.f;
            float viewX = viewX2 - idim.x; 

            float inverseZ = (viewX2 > 1e-6);
            float viewZid = !inverseZ ? z : max(z-1.f, 1e-6f);
            float viewZ2id = inverseZ ? z : max(z-1.f, 1e-6f);
            float viewZ = hNFratio * dimf.z/(viewZid);
            float viewZ2 = hNFratio * dimf.z/(viewZ2id);

            float viewYbase = y*idim.y - 1.f;
            bool inverseY = inverseZ != (viewYbase >= 0.f);
            float viewY = viewYbase - (!inverseY ? idim.y : 0.f);
            float viewY2 = viewYbase - (inverseY ? idim.y : 0.f);

            vec3 p1 = vec3(viewX * ipm[0].x, viewY * ipm[1].y, -1)/(viewZ*ipm[2].w);
            vec3 p2 = vec3(viewX2 * ipm[0].x, viewY2 * ipm[1].y, -1)/(viewZ2*ipm[2].w);

            AABBs[aabbID].first = min(p1, p2);
            AABBs[aabbID].second = max(p1, p2);
        }
    }

    
    #ifdef PER_LIGHT_OPTIMIZED_CLUSTERED_RENDERING
        /****** Per Light Optimized Culling ******/

        int buffSize = MAX_LIGHT_PER_CLUSTER*dim.x*dim.y*dim.z;

        for(int i = 0; i < buffSize; i+= MAX_LIGHT_PER_CLUSTER) 
            buff[i] = lmax;
        
        if(cameraIsORtho)
        {
            for(int i = 0; i < lmax; i++)
            {
                const LightInfos &l = lbuffv[i];
                
                float maxXid = dim.x;
                float maxYid = dim.y;
                float maxZid = dim.z;

                float minXid = 0;
                float minYid = 0;
                float minZid = 0;

                for(int x = minXid; x < maxXid; x++)
                for(int y = minYid; y < maxYid; y++)
                for(int z = minZid; z < maxZid; z++)
                {
                    aabbID = x*dim.z*dim.y + y*dim.z + z;
                    vec3 minP = AABBs[aabbID].first;
                    vec3 maxP = AABBs[aabbID].second;

                    const vec3 p = vec3(l._position);
                    const vec3 closest = p-max(minP, min(maxP, p));
                    bool culled = dot(closest, closest) <= l._direction.x*l._direction.x;

                    if(culled)
                    {
                        int cid = MAX_LIGHT_PER_CLUSTER*aabbID;
                        const int maxCID = cid + MAX_LIGHT_PER_CLUSTER - 1;

                        while(buff[cid] != lmax && cid < maxCID) cid++;

                        buff[cid] = i;
                        buff[cid+1] = lmax;
                    }
                }
            }
        }
        else
        for(int i = 0; i < lmax; i++)
        {
            const LightInfos &l = lbuffv[i];

            if(l._infos.a != POINT_LIGHT) continue;

            float minDepth = -0.5*(l._position.z + l._direction.x)/far_;
            int minZid = clamp((int)(minDepth*dim.z), 0, dim.z);

            float maxDepth = -0.5*(l._position.z - l._direction.x)/far_;
            int maxZid = clamp((int)(maxDepth*dim.z)+1, 0, dim.z);
            
            if(!maxZid && !cameraIsORtho) continue;

            float depthFront = 1.f/(l._position.z);

            float minSphereX = (l._position.x - l._direction.x);
            float minX = minSphereX*pm[0][0]*depthFront;
            minX = minX*-0.5 + 0.5;
            int minXid = clamp((int)(minX*dim.x) - (minSphereX > 0.f ? 0 : 1), 0, dim.x);

            float maxSphereX = (l._position.x + l._direction.x);
            float maxX = maxSphereX*pm[0][0]*depthFront;
            maxX = maxX*-0.5 + 0.5;
            int maxXid = clamp((int)(maxX*dim.x) + (maxSphereX > 0.f ? 2 : 1), 0, dim.x);

            float minSphereY = (l._position.y - l._direction.x);
            float minY = minSphereY*pm[1][1]*depthFront;
            minY = minY*-0.5 + 0.5;
            int minYid = clamp((int)(minY*dim.y) - (minSphereY > 0.f ? 0 : 1), 0, dim.y);

            float maxSphereY = (l._position.y + l._direction.x);
            float maxY = maxSphereY*pm[1][1]*depthFront;
            maxY = maxY*-0.5 + 0.5;
            int maxYid = clamp((int)(maxY*dim.y) + (maxSphereY > 0.f ? 2 : 1), 0, dim.y);

            if(l._position.z + l._direction.x > -near_)
            {
                minYid = 0; maxYid = dim.y; minXid = 0; maxXid = dim.x;
            }

            for(int x = minXid; x < maxXid; x++)
            for(int y = minYid; y < maxYid; y++)
            for(int z = minZid; z < maxZid; z++)
            {
                aabbID = x*dim.z*dim.y + y*dim.z + z;
                vec3 minP = AABBs[aabbID].first;
                vec3 maxP = AABBs[aabbID].second;

                const vec3 p = vec3(l._position);
                const vec3 closest = p-max(minP, min(maxP, p));
                bool culled = dot(closest, closest) <= l._direction.x*l._direction.x;

                if(culled)
                {
                    int cid = MAX_LIGHT_PER_CLUSTER*aabbID;
                    const int maxCID = cid + MAX_LIGHT_PER_CLUSTER - 1;

                    while(buff[cid] != lmax && cid < maxCID) cid++;

                    buff[cid] = i;
                    buff[cid+1] = lmax;
                }
            }
        }
    #else
        /****** Per Cluster Culling ******/
        int clusterOff = 0;
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
    #endif

    delete [] lbuffv;
}

uint Scene::draw(int layer)
{
    // NOTIF_MESSAGE(
    //     groups.size(), "   ", staticGroups.size()
    // )

    // static BenchTimer tmp;
    
    callsTime.start();
    // drawcnt = 0;

    for(auto &i : meshes)
        drawcnt += i.draw(layer, useBindlessTextures);
    
    // tmp.start();

    for(auto &i : unsortedMeshes)
        if(i->isCulled(layer))
        {
            if(useBindlessTextures)
                i->setBindlessMaps();
            else
                i->bindAllMaps();
            
            i->getMaterial()->activate();
            drawcnt += i->drawVAO(layer);
            i->getMaterial()->deactivate();
        }

    // tmp.stop();

    lightBuffer.reset();
    callsTime.hold();
    
    // std::cout << tmp.getLastAvg() << "\n";

    glFlush();
    if(layer == 0 and tree.root.recursiveElementCounter)
    {
        occlusionCullingTime.start();
        App::occlusionPass.render(*globals.currentCamera);
        occlusionCullingTime.stop();
    }
    
    return drawcnt;
}

uint Scene::getDrawCalls(){return drawcnt;}

uint Scene::getPolyCount()
{
    int cnt = 0;

    for(auto &mg : meshes)
        for(auto &m : mg.meshes)
            if(m->defaultMode == GL_TRIANGLES)
                if(m->state.hide != ModelStatus::HIDE && m->isCulled(0) && m->getVao() && m->getVao()->attributes.size())
                    {
                        if(m->getVao().nbFaces)
                            cnt += m->getVao().nbFaces;
                        else
                            cnt += m->getVao()->attributes[0].getVertexCount()/3;
                    }

    for(auto &m : unsortedMeshes)
        if(m->defaultMode == GL_TRIANGLES)
            if(m->state.hide != ModelStatus::HIDE && m->isCulled(0) && m->getVao() && m->getVao()->attributes.size())
            {
                if(m->getVao().nbFaces)
                    cnt += m->getVao().nbFaces;
                else
                    cnt += m->getVao()->attributes[0].getVertexCount()/3;
            }

    return cnt;
}

uint Scene::getVertexCount()
{
    int cnt = 0;

    for(auto &mg : meshes)
        for(auto &m : mg.meshes)
        if(m->state.hide != ModelStatus::HIDE && m->isCulled(0) && m->getVao() && m->getVao()->attributes.size())
            {
                auto &a = m->getVao()->attributes[0];
                cnt += a.getVertexCount();
            }

    for(auto &m : unsortedMeshes)
        if(m->state.hide != ModelStatus::HIDE && m->isCulled(0) && m->getVao() && m->getVao()->attributes.size())
        {
            auto &a = m->getVao()->attributes[0];
            cnt += a.getVertexCount();
        }

    return cnt;
}

uint Scene::getMaterialCount()
{
    int cnt = 0;

    for(auto &mg : meshes)
        for(auto &m : mg.meshes)
            if(m->state.hide != ModelStatus::HIDE)
            {
                cnt ++;
                break;
            }

    for(auto &m : unsortedMeshes)
        if(m->state.hide != ModelStatus::HIDE)
        {
            cnt ++;
        }

    return cnt;
}

uint Scene::getTotalMeshes()
{
    int cnt = 0;

    for(auto &mg : meshes)
        cnt += mg.meshes.size();

    cnt += unsortedMeshes.size();

    return cnt;
}

uint Scene::getShadowMapCount()
{
    int cnt = 0;

    for(auto &l : lights)
        if(l->getInfos()._infos.b&LIGHT_SHADOW_ACTIVATED)
            cnt++;

    return cnt;
}

// TODO : Add a way to have custom depth vertex shader to some object
// NOTE : If an object have a non trivial vertex shader, maybe the vertices
//        should be calculated just once in a compute/cpu prepass. But this 
//        woulnd't be compatible with tesselation.

void Scene::depthOnlyDraw(int layer, Camera *camera, bool doCulling, int layers)
{
    // if(doCulling) this->cull();

    depthOnlyCallsTime.start();

    if(depthOnlyMaterial)
    {
        for(auto i = meshes.begin(); i != meshes.end(); i++)
        {

            // if(i->material.depthOnly)
            //     i->material.depthOnly->activate();
            // else
            //     depthOnlyMaterial->activate();
            
            ShaderProgram *dom = i->material.depthOnly ? i->material.depthOnly.get() : depthOnlyMaterial.get();
            dom->activate();

            ShaderUniform(camera->getProjectionViewMatrixAddr(), 2).activate();
            ShaderUniform(camera->getViewMatrixAddr(), 3).activate();
            ShaderUniform(camera->getProjectionMatrixAddr(), 4).activate();
            // ShaderUniform(camera.getPositionAddr(), 5).activate();
            // ShaderUniform(camera.getDirectionAddr(), 6).activate();

            if(layers > 0)
            for(int i = 0; i < layers; i++)
                ShaderUniform(camera[i].getProjectionViewMatrixAddr(), 48 + i).activate();

            // if(cull)
            // {
            //     for(auto j : i->meshes)
            //         if(j->isCulled(layer))
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
                for(auto j : i->staticMeshes)
                    if(j->isCulled(layer))
                    {
                        j->setBindlessMaps();
                        drawcnt += j->drawVAO(layer, true);
                    }

                for(auto j : i->meshes)
                    if(j->isCulled(layer))
                    {
                        j->setBindlessMaps();
                        drawcnt += j->drawVAO(layer, true);
                    }
            }
            else
            {
                for(auto j : i->staticMeshes)
                    if(j->isCulled(layer))
                    {
                        j->setBindlessMaps();
                        drawcnt += j->drawVAO(layer, true);
                    }

                for(auto j : i->meshes)
                    if(j->isCulled(layer))
                    {
                        j->bindAllMaps();
                        drawcnt += j->drawVAO(layer, true);
                    }
            }

            dom->deactivate();
        }

        depthOnlyMaterial->deactivate();
    }

    glFlush();
    
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

   depthOnlyCallsTime.hold();
}

void Scene::generateShadowMaps()
{
    shadowPassCallsTime.start();
    Camera *tmp = globals.currentCamera;

    int layer = 1;

    for(auto i : lights)
        if(
            i->getInfos()._color.a >= 1e-6 
            && i->getInfos()._infos.b&LIGHT_SHADOW_ACTIVATED
            && i->shadowCameraSize != vec2(0)
        )
        {
            i->shadowMap.activate();
            globals.currentCamera = tmp;
            i->updateShadowCamera();
            i->shadowCamera[0].updateFrustum();
            globals.currentCamera = &i->shadowCamera[0];
            
            // shadowPassCallsTime.hold();
            // cull(layer);
            // shadowPassCallsTime.start();

            // ShaderUniform(i->shadowCamera[0].getProjectionMatrixAddr(), 48).activate();
            // ShaderUniform(i->shadowCamera[1].getProjectionMatrixAddr(), 49).activate();
            // ShaderUniform(i->shadowCamera[2].getProjectionMatrixAddr(), 50).activate();
            depthOnlyDraw(layer, i->shadowCamera, false, 3);
            i->shadowMap.deactivate();

            shadowPassCallsTime.hold();
            cull(layer, 3);
            shadowPassCallsTime.start();

            layer++;
        }
    shadowPassCallsTime.hold();
}

void Scene::remove(ModelRef mesh)
{
    for(auto i = meshes.begin(); i != meshes.end(); i++)
    {
        if(i->material.get() == mesh->getMaterial().get())
        {
            for(auto j = i->meshes.begin(); j != i->meshes.end(); j++)
                if(j->get() == mesh.get())
                {
                    i->meshes.erase(j);

                    if(!i->meshes.size())
                        meshes.erase(i);
                    
                    return;
                }

            for(auto j = i->staticMeshes.begin(); j != i->staticMeshes.end(); j++)
                if(j->get() == mesh.get())
                {
                    i->staticMeshes.erase(j);

                    if(!i->meshes.size() and !i->staticMeshes.size())
                        meshes.erase(i);
                    
                    return;
                }
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
    tree.remove(group);

    removeGroupElement(group);

    for(auto i = groups.begin(); i != groups.end(); i++)
        if(i->get() == group.get())
        {
            groups.erase(i);
            return;
        }
    
    for(auto i = staticGroups.begin(); i != staticGroups.end(); i++)
        if(i->get() == group.get())
        {
            staticGroups.erase(i);
            return;
        }

    for(auto &m : group->getMeshes())
    {
        remove(m);
    }

    for(auto &l : group->getLights())
    {
        remove(l);
    }

    for(auto &c : group->getChildren())
    {
        remove(c);
    }
}

void Scene::cull(int layer, int cameraLayers)
{
    cullTime.start();

    // static int cnttmp = 0;
    // cnttmp ++;
    // if(cnttmp%3 == 0)
    //     system("clear");

    // if(tr)

    if(tree.root.recursiveElementCounter)
    {
    //     system("clear");

    //     static NAMED_TIMER(treeCull)
    //     treeCull.start();
        
        uint cullCall = tree.root.cull(layer, cameraLayers);

        if(layer == 0) tree.root.updateOcclusion();
    
    //     treeCull.stop();
    
    //     std::cout << treeCull;
    //     std::cout << tree.root.nodesCount() << "\n";
    //     std::cout << cullCall << "\n";
    }

    for(auto i = meshes.begin(); i != meshes.end(); i++)
    {
        for(auto j : i->staticMeshes)
            j->cull(layer, cameraLayers);

        for(auto j : i->meshes)
            j->cull(layer, cameraLayers);
    }

    for(auto i : unsortedMeshes)
        i->cull(layer, cameraLayers);
    
    cullTime.hold();
}

void Scene::activateClusteredLighting(ivec3 dimention, float vFar)
{
    useClusteredLighting = true;
    clusteredLight.allocate(dimention);
    clusteredLight.send();

    clusteredLight.vFar = vFar;
    clusteredLight.ivFar = 0.5 * globals.currentCamera->getState().nearPlane / vFar;
}

void Scene::deactivateClusteredLighting()
{
    useClusteredLighting = false;
}

void Scene::endTimers()
{
    cullTime.start();
    cullTime.stop();
    callsTime.start();
    callsTime.stop();
    depthOnlyCallsTime.start();
    depthOnlyCallsTime.stop();
    shadowPassCallsTime.start();
    shadowPassCallsTime.stop();
    lightBufferTime.start();
    lightBufferTime .stop();
}


void Scene::activateTreeCulling(vec3 worldSize)
{
    tree.worldSize = worldSize;
}

// void isIn(float minx, float maxx, float maxs, float mins)
// {
//     return minx 
// }

std::string printCoords(StaticSceneOctree::ID id)
{
    std::stringstream ss;
    ss << "( ";
    for(auto i : id.coord)
        ss << (int)i << " ";

    ss << ")";
    return ss.str();
}

StaticSceneOctree::ID StaticSceneOctree::getID(vec3 min, vec3 max)
{
    ID id;
    bool finished = false;

    vec3 middle = worldCenter;
    float asize = 0.5f;
    vec3 size = worldSize*(asize + bonusSpace*2.f);

    // NOTIF_MESSAGE(min, "   ", max)

    for(int j = 0; j < QUADTREE_ITERATION and !finished; j++)
    {
        vec3 middle1;
        vec3 middle2;
        
        if(j)
        {
            middle1 = middle-size*0.5f;
            middle2 = middle+size*0.5f;
        }
        else
        {
            middle1 = middle-worldSize*0.25f;
            middle2 = middle+worldSize*0.25f;
        }

        id.coord[j] = 0;

        for(int i = 0; i < 3 and !finished; i++)
        {
            if(min[i] > middle1[i]-size[i]*0.5f and max[i] < middle1[i]+size[i]*0.5f)
            {
                // id.coord[j] |= 0;
                middle[i] = middle1[i];
            }
            else
            if(min[i] > middle2[i]-size[i]*0.5f and max[i] < middle2[i]+size[i]*0.5f)
            {
                id.coord[j] |= 1 << i;
                middle[i] = middle2[i];
            }
            else
            {
                finished = true;
                id.coord[j] = -1;
                // ERROR_MESSAGE(j)
                break;
            }
        }

        if(finished) continue;

        // WARNING_MESSAGE(
        //     "\n\t", middle, "\t", size, "\t", (int)id.coord[j],
        //     "\n\t", middle-size*0.5f, "\t", middle+size*0.5f
        // )

        asize *= 0.5f;

        size = worldSize*(asize + bonusSpace*2.f);
        // size = worldSize*(asize + asize*bonusSpace*2.f);
        // size = size*(0.5f + bonusSpace*2.f);
    }

    // NOTIF_MESSAGE(printCoords(id))

    return  id;
}


StaticSceneOctree::ID StaticSceneOctree::add(ObjectGroupRef group)
{
    root.middle = worldCenter;
    root.size = worldSize;

    auto box = group->getMeshesBoundingBox();
    auto id = getID(box.first, box.second);

    root.add(group, id, 0);

    return id;
}


bool StaticSceneOctree::Node::add(ObjectGroupRef group, StaticSceneOctree::ID id, int depth)
{
    if(id.coord[depth] == -1 || depth == QUADTREE_ITERATION-1)
    {
        for(auto i : elements)
            if(i.get() == group.get())
                return false;

        group->staticSceneID = id;
        elements.push_back(group);

        group->propagateExternCullPtr(drawElements);

        if(!query)
            query = App::occlusionPass.add(middle, size, doOcclusionQuerry);

        return true;
    }

    if(!childs[id.coord[depth]])
    {
        childs[id.coord[depth]] = std::make_shared<Node>();

        childs[id.coord[depth]]->size = size*0.5f + 2.f*StaticSceneOctree::worldSize*StaticSceneOctree::bonusSpace;

        // childs[id.coord[depth]]->size = size*(0.5f + 2.f*StaticSceneOctree::bonusSpace);



        vec3 signs(0);
        for(int i = 0; i < 3; i++)
            signs[i] = id.coord[depth] & 1<<i ? +1.f : -1.f;

        childs[id.coord[depth]]->middle = middle + signs*size*0.25f;

        childs[id.coord[depth]]->nodeDepth = nodeDepth+1;

        // childs[id.coord[depth]]->query = App::occlusionPass.add(
        //     childs[id.coord[depth]]->middle,
        //     childs[id.coord[depth]]->size,
        //     // (bool *)childs[id.coord[depth]]->status
        //     childs[id.coord[depth]]->doOcclusionQuerry
        // );
    }

    if(childs[id.coord[depth]]->add(group, id, depth+1))
    {
        recursiveElementCounter ++;
        return true;
    };
    return false;
}

bool StaticSceneOctree::remove(ObjectGroupRef group)
{
    root.middle = worldCenter;
    root.size = worldSize;

    auto box = group->getMeshesBoundingBox();
    // auto id = getID(box.first, box.second);

    return root.remove(group, group->staticSceneID, 0);
}



bool StaticSceneOctree::Node::remove(ObjectGroupRef group, StaticSceneOctree::ID id, int depth)
{
    // recursiveElementCounter ++;


    // for(auto i : childs)
    // if(i)
    // {
    //     if(i->remove(group, id, depth+1))
    //         return true;
    // }

    // for(auto i = elements.begin(); i != elements.end(); i++)
    // {
    //     if(i->get() == group.get())
    //     {
    //         elements.erase(i);
    //         return true;
    //     }
    // }

    // return false;

    

    if(id.coord[depth] == -1 or depth == QUADTREE_ITERATION-1)
    {
        if(elements.empty())
            return false;

        for(auto i = elements.begin(); i != elements.end(); i++)
            if(i->get() == group.get())
            {
                elements.erase(i);
                return true;
            }

        // WARNING_MESSAGE("Cannot remove elements from octree-node at depth ", depth, " and coords ", printCoords(id))
        // ERROR_MESSAGE(elements.size())

        return false;
    }

    if(!childs[id.coord[depth]])
    {
        WARNING_MESSAGE("Cannot remove element from octree, path doesn't exist at depth ", depth)
        return false;
    }

    if(childs[id.coord[depth]]->remove(group, id, depth+1))
    {
        recursiveElementCounter --;

        // if(childs[id.coord[depth]]->elements.empty())
        //     childs[id.coord[depth]] = std::shared_ptr<Node>();

        return true;
    }

    // if(childs[id.coord[depth]]->elements.empty())
    //     childs[id.coord[depth]] = std::shared_ptr<Node>();
    return false;
}

std::string StaticSceneOctree::Node::toStr()
{
    std::stringstream ss;

    std::string tab;

    for(int i = 0; i < nodeDepth; i++) tab += '\t';

    ss  <<
    tab << "Node\n" << 
    tab << "\tmin : " << middle-size*0.5f << "\n" <<
    tab << "\tmax : " << middle+size*0.5f << "\n" <<
    tab << "\ttotal elements nb : " << recursiveElementCounter << "\n" <<
    tab << "\tnode elements nb  : " << elements.size() << "\n" <<
    tab << "\tchildren nodes nb : " << nodesCount() << "\n" <<
    // tab << "\telements : " << elements.size() << "\n" <<
    TERMINAL_TIMER;

    tab += '\t';

    // for(auto i : elements)
    //     ss  << 
    //     tab << i->name << "\n" <<
    //     tab << "\tmin" << i->getMeshesBoundingBox().first << "\n" <<
    //     tab << "\tmax" << i->getMeshesBoundingBox().second << "\n";

    ss << TERMINAL_RESET;

    for(auto i : childs)
        if(i)
            ss << i->toStr();

    return ss.str();
}

std::string StaticSceneOctree::toStr()
{
    return root.toStr();
}

int StaticSceneOctree::Node::nodesCount()
{
    int cnt = 1;
    for(auto i : childs)
        if(i)
            cnt += i->nodesCount();

    return cnt;
}

void StaticSceneOctree::Node::recusriveCall(std::function<void(Node &node)> f)
{
    f(*this);
    for(auto i : childs) if(i) i->recusriveCall(f);
};


vec3 applyPlanetCurvature(vec3 point)
{
    static float iPlanetSize2 = 1.f/(globals.planetSize() * globals.planetSize());

    vec2 pc = vec2(point.x, point.z) - vec2(globals.playerCamPos().x, globals.playerCamPos().z);
    point.y += globals.planetSize() * (sqrt(1.f - dot(pc, pc)*iPlanetSize2) - 1.f);

    return point;
}

/*
    https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
*/
bool isOnForwardPlane(const FrustumPlane &plane, const vec3 &extents, vec3 center)
{
    const float r = (
        extents.x*abs(plane.normal.x) +
        extents.y*abs(plane.normal.y) +
        extents.z*abs(plane.normal.z)
    );

    center = applyPlanetCurvature(center);

    return -r <= dot(plane.normal, center) - plane.distance;
}

bool isOnFrustum(const Frustum &frustum, const vec3 &extents, const vec3 &center)
{
    return 
        isOnForwardPlane(frustum.left, extents, center)
        and
        isOnForwardPlane(frustum.right, extents, center)
        and
        isOnForwardPlane(frustum.bottom, extents, center)
        and
        isOnForwardPlane(frustum.top, extents, center)
        and
        isOnForwardPlane(frustum.near_, extents, center)
        and
        isOnForwardPlane(frustum.far_, extents, center)
    ;
}

bool isOnForwardPlane(const FrustumPlane &plane, vec3 point)
{
    point = applyPlanetCurvature(point);

    // return dot(plane.normal, point) - plane.distance > 0;
    return dot(plane.normal, point - plane.position) > 0;
}

int isFullyInsideFrustum(const Frustum &frustum, const vec3 &e, const vec3 &center)
{    
    return 

    (
        isOnForwardPlane(frustum.left,   center + vec3(+e.x, +e.y, +e.z)) and
        isOnForwardPlane(frustum.right,  center + vec3(+e.x, +e.y, +e.z)) and
        isOnForwardPlane(frustum.bottom, center + vec3(+e.x, +e.y, +e.z)) and
        isOnForwardPlane(frustum.top,    center + vec3(+e.x, +e.y, +e.z)) and
        isOnForwardPlane(frustum.near_,  center + vec3(+e.x, +e.y, +e.z)) and
        isOnForwardPlane(frustum.far_,   center + vec3(+e.x, +e.y, +e.z))
    ) +

    (
        isOnForwardPlane(frustum.left,   center + vec3(+e.x, +e.y, -e.z)) and
        isOnForwardPlane(frustum.right,  center + vec3(+e.x, +e.y, -e.z)) and
        isOnForwardPlane(frustum.bottom, center + vec3(+e.x, +e.y, -e.z)) and
        isOnForwardPlane(frustum.top,    center + vec3(+e.x, +e.y, -e.z)) and
        isOnForwardPlane(frustum.near_,  center + vec3(+e.x, +e.y, -e.z)) and
        isOnForwardPlane(frustum.far_,   center + vec3(+e.x, +e.y, -e.z))
    ) +

    (
        isOnForwardPlane(frustum.left,   center + vec3(+e.x, -e.y, +e.z)) and
        isOnForwardPlane(frustum.right,  center + vec3(+e.x, -e.y, +e.z)) and
        isOnForwardPlane(frustum.bottom, center + vec3(+e.x, -e.y, +e.z)) and
        isOnForwardPlane(frustum.top,    center + vec3(+e.x, -e.y, +e.z)) and
        isOnForwardPlane(frustum.near_,  center + vec3(+e.x, -e.y, +e.z)) and
        isOnForwardPlane(frustum.far_,   center + vec3(+e.x, -e.y, +e.z)) 
    ) +
    (
        isOnForwardPlane(frustum.left,   center + vec3(+e.x, -e.y, -e.z)) and
        isOnForwardPlane(frustum.right,  center + vec3(+e.x, -e.y, -e.z)) and
        isOnForwardPlane(frustum.bottom, center + vec3(+e.x, -e.y, -e.z)) and
        isOnForwardPlane(frustum.top,    center + vec3(+e.x, -e.y, -e.z)) and
        isOnForwardPlane(frustum.near_,  center + vec3(+e.x, -e.y, -e.z)) and
        isOnForwardPlane(frustum.far_,   center + vec3(+e.x, -e.y, -e.z)) 
    ) +
    (
        isOnForwardPlane(frustum.left,   center + vec3(-e.x, +e.y, +e.z)) and
        isOnForwardPlane(frustum.right,  center + vec3(-e.x, +e.y, +e.z)) and
        isOnForwardPlane(frustum.bottom, center + vec3(-e.x, +e.y, +e.z)) and
        isOnForwardPlane(frustum.top,    center + vec3(-e.x, +e.y, +e.z)) and
        isOnForwardPlane(frustum.near_,  center + vec3(-e.x, +e.y, +e.z)) and
        isOnForwardPlane(frustum.far_,   center + vec3(-e.x, +e.y, +e.z))
    ) +
    (

        isOnForwardPlane(frustum.left,   center + vec3(-e.x, +e.y, -e.z)) and
        isOnForwardPlane(frustum.right,  center + vec3(-e.x, +e.y, -e.z)) and
        isOnForwardPlane(frustum.bottom, center + vec3(-e.x, +e.y, -e.z)) and
        isOnForwardPlane(frustum.top,    center + vec3(-e.x, +e.y, -e.z)) and
        isOnForwardPlane(frustum.near_,  center + vec3(-e.x, +e.y, -e.z)) and
        isOnForwardPlane(frustum.far_,   center + vec3(-e.x, +e.y, -e.z))
    ) +
    (
        isOnForwardPlane(frustum.left,   center + vec3(-e.x, -e.y, +e.z)) and
        isOnForwardPlane(frustum.right,  center + vec3(-e.x, -e.y, +e.z)) and
        isOnForwardPlane(frustum.bottom, center + vec3(-e.x, -e.y, +e.z)) and
        isOnForwardPlane(frustum.top,    center + vec3(-e.x, -e.y, +e.z)) and
        isOnForwardPlane(frustum.near_,  center + vec3(-e.x, -e.y, +e.z)) and
        isOnForwardPlane(frustum.far_,   center + vec3(-e.x, -e.y, +e.z))
    ) +
    (
        isOnForwardPlane(frustum.left,   center + vec3(-e.x, -e.y, -e.z)) and
        isOnForwardPlane(frustum.right,  center + vec3(-e.x, -e.y, -e.z)) and
        isOnForwardPlane(frustum.bottom, center + vec3(-e.x, -e.y, -e.z)) and
        isOnForwardPlane(frustum.top,    center + vec3(-e.x, -e.y, -e.z)) and
        isOnForwardPlane(frustum.near_,  center + vec3(-e.x, -e.y, -e.z)) and
        isOnForwardPlane(frustum.far_,   center + vec3(-e.x, -e.y, -e.z))
    )
    ;
}

void StaticSceneOctree::Node::propagateFullyVisible(int layer)
{
    // if(status == FullyVisible) return;


    // bool isLeaf = true;
    // for(auto i : childs) if(i) isLeaf = false;
    // doOcclusionQuerry[layer] = isLeaf;

    doOcclusionQuerry[layer] = true;
    // drawElements[layer] = true;

    if(status[layer] != FullyVisible)
    {
        status[layer] = FullyVisible;
        drawElements[layer] = true;
    
        // for(auto i : elements)
        //     i->propagateExternCullMode(layer, true);

        for(auto i : childs)
            if(i)
                i->propagateFullyVisible(layer);
    }


}

void StaticSceneOctree::Node::propagateNotVisible(int layer)
{
    doOcclusionQuerry[layer] = false;

    if(status[layer] != NotVisible)
    {
        status[layer] = NotVisible;
        drawElements[layer] = false;
    
        for(auto i : elements)
            i->propagateExternCullMode(layer, false);

        for(auto i : childs)
            if(i)
                i->propagateNotVisible(layer);
    }

}

uint StaticSceneOctree::Node::cull(int layer, int cameraLayers)
{
    uint cnt = 1;

    const vec3 hsize = size*0.5f;

    vec3 points[8] = 
    {
        applyPlanetCurvature(middle - vec3(+hsize.x, +hsize.y, +hsize.z)),
        applyPlanetCurvature(middle - vec3(+hsize.x, +hsize.y, -hsize.z)),
        applyPlanetCurvature(middle - vec3(+hsize.x, -hsize.y, +hsize.z)),
        applyPlanetCurvature(middle - vec3(+hsize.x, -hsize.y, -hsize.z)),
        applyPlanetCurvature(middle - vec3(-hsize.x, +hsize.y, +hsize.z)),
        applyPlanetCurvature(middle - vec3(-hsize.x, +hsize.y, -hsize.z)),
        applyPlanetCurvature(middle - vec3(-hsize.x, -hsize.y, +hsize.z)),
        applyPlanetCurvature(middle - vec3(-hsize.x, -hsize.y, -hsize.z))
    };

    bool onFrustum = false;
    bool fulyOnScreen = false;

    for(int l = 0; l < cameraLayers; l ++)
    {
        const mat4& matrix = *globals.currentCamera[l].getProjectionViewMatrixAddr();
    
        vec3 proj[8];
    
        for(int i = 0; i < 8; i++)
        {
            vec4 tmp = matrix * vec4(points[i], 1.0);
            proj[i] = vec3(tmp) / max(tmp.w, 0.0000000001f);
        }
    
        vec3 maxProj = proj[0];
        vec3 minProj = proj[0];
        for(int i = 1; i < 8; i++)
        {
            maxProj = max(maxProj, proj[i]);
            minProj = min(minProj, proj[i]);
        }
    
        bool onFrustum2 =
        (
            maxProj.z > +0.f and
            maxProj.x > -1.f and maxProj.y > -1.f and
            minProj.x < +1.f and minProj.y < +1.f
        );
    
        bool fulyOnScreen2 = onFrustum2 and
        (
            maxProj.x < +1.f and maxProj.y < +1.f and
            minProj.x > -1.f and minProj.y > -1.f
        );

        onFrustum |= onFrustum2;
        fulyOnScreen |= fulyOnScreen2;
    }
    
    switch (onFrustum + fulyOnScreen) 
    {
        case 0 : 
            propagateNotVisible(layer);
        break;

        case 2 :
            propagateFullyVisible(layer);
        break;

        case 1 : //if(status == CullingStatus::PartiallyVisible) return;

            // if(status[layer] != CullingStatus::PartiallyVisible)
                // for(auto i : elements)
                // {
                //     i->propagateExternCullMode(layer, true);
                //     // i->disableExternCullMode();
                // }
            
            drawElements[layer] = true;

            status[layer] = CullingStatus::PartiallyVisible;
            doOcclusionQuerry[layer] = true;

            for(auto i : childs)
                if(i)
                    cnt += i->cull(layer, cameraLayers);
        break;

        default : break;
    }

    return cnt;
}

void StaticSceneOctree::Node::updateOcclusion()
{
    // return;
    
    // if(!nodeDepth) std::cout << "================";

    // if(!drawElements[0]) return;
    if(!doOcclusionQuerry[0]) return;

    if(query)
    {
        query->retreiveQueryResults();
    
        drawElements[0] = query->getQueryResult() > 0;

        // if(!drawElements[0])
        // {
        //     std::cout << "OMG JUST OCCLUDED " << elements.size() << " ELEMENTS!!!\n";
        // }
    }

    for(auto i : childs)
        if(i)
            i->updateOcclusion();
}