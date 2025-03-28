#include <Graphics/Scene.hpp>
#include <iostream>
#include <Globals.hpp> 
#include <Helpers.hpp>
#include <MathsUtils.hpp>

#define PER_LIGHT_OPTIMIZED_CLUSTERED_RENDERING

Scene::Scene()
{
    lightBuffer.send();
}

Scene& Scene::add(ModelRef mesh, bool sort)
{
    if(!sort || !mesh->sorted)
    {
        mesh->sorted = false;
        
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
        for(auto &i : meshes)
            if(i->isCulled())
            {
                i->setBindlessMaps();
                drawcnt += i->drawVAO();
            }
    }
    else
    {
        for(auto &i : meshes)
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
    drawcnt = 0;
    
    for(auto &i : groups)
        i->update();
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
    const float near = globals.currentCamera->getState().nearPlane;
    const float far = clusteredLight.vFar;

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

    if(AABBs.empty())
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

        for(int i = 0; i < lmax; i++)
        {
            const LightInfos &l = lbuffv[i];

            if(l._infos.a != POINT_LIGHT) continue;

            float minDepth = -0.5*(l._position.z + l._direction.x)/far;
            int minZid = clamp((int)(minDepth*dim.z), 0, dim.z);

            float maxDepth = -0.5*(l._position.z - l._direction.x)/far;
            int maxZid = clamp((int)(maxDepth*dim.z)+1, 0, dim.z);
            
            if(!maxZid) continue;

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

            if(l._position.z + l._direction.x > -near)
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

uint Scene::draw()
{
    callsTime.start();
    // drawcnt = 0;

    for(auto &i : meshes)
        drawcnt += i.draw(useBindlessTextures);
    
    for(auto &i : unsortedMeshes)
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
    callsTime.hold();
    return drawcnt;
}

uint Scene::getDrawCalls(){return drawcnt;}

uint Scene::getPolyCount()
{
    int cnt = 0;

    for(auto &mg : meshes)
        for(auto &m : mg.meshes)
            if(m->defaultMode == GL_TRIANGLES)
                if(m->state.hide != ModelStatus::HIDE && m->isCulled() && m->getVao() && m->getVao()->attributes.size())
                    {
                        if(m->getVao().nbFaces)
                            cnt += m->getVao().nbFaces;
                        else
                            cnt += m->getVao()->attributes[0].getVertexCount()/3;
                    }

    for(auto &m : unsortedMeshes)
        if(m->defaultMode == GL_TRIANGLES)
            if(m->state.hide != ModelStatus::HIDE && m->isCulled() && m->getVao() && m->getVao()->attributes.size())
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
        if(m->state.hide != ModelStatus::HIDE && m->isCulled() && m->getVao() && m->getVao()->attributes.size())
            {
                auto &a = m->getVao()->attributes[0];
                cnt += a.getVertexCount();
            }

    for(auto &m : unsortedMeshes)
        if(m->state.hide != ModelStatus::HIDE && m->isCulled() && m->getVao() && m->getVao()->attributes.size())
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

void Scene::depthOnlyDraw(Camera &camera, bool doCulling)
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

            ShaderUniform(camera.getProjectionViewMatrixAddr(), 2).activate();
            ShaderUniform(camera.getViewMatrixAddr(), 3).activate();
            ShaderUniform(camera.getProjectionMatrixAddr(), 4).activate();
            // ShaderUniform(camera.getPositionAddr(), 5).activate();
            // ShaderUniform(camera.getDirectionAddr(), 6).activate();

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
                    if(j->isCulled())
                    {
                        j->setBindlessMaps();
                        drawcnt += j->drawVAO(true);
                    }
            }
            else
            {
                for(auto j : i->meshes)
                    if(j->isCulled())
                    {
                        j->bindAllMaps();
                        drawcnt += j->drawVAO(true);
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

   depthOnlyCallsTime.hold();
}

void Scene::generateShadowMaps()
{
    shadowPassCallsTime.start();
    Camera *tmp = globals.currentCamera;

    for(auto i : lights)
        if(i->getInfos()._color.a >= 1e-6 && i->getInfos()._infos.b&LIGHT_SHADOW_ACTIVATED)
        {
            i->shadowMap.activate();
            globals.currentCamera = tmp;
            i->updateShadowCamera();
            i->shadowCamera.updateFrustum();
            globals.currentCamera = &i->shadowCamera;
            cull();
            depthOnlyDraw(i->shadowCamera);
            i->shadowMap.deactivate();
        }
    shadowPassCallsTime.hold();
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

                    if(!i->meshes.size())
                        meshes.erase(i);
                    
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

void Scene::cull()
{
    cullTime.start();
    
    for(auto i = meshes.begin(); i != meshes.end(); i++)
    {
        for(auto j : i->meshes)
            j->cull();
    }

    for(auto i : unsortedMeshes)
        i->cull();
    
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
    cullTime.end();
    callsTime.start();
    callsTime.end();
    depthOnlyCallsTime.start();
    depthOnlyCallsTime.end();
    shadowPassCallsTime.start();
    shadowPassCallsTime.end();
    lightBufferTime.start();
    lightBufferTime .end();
}
