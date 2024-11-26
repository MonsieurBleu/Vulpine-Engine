#include <Graphics/ObjectGroup.hpp>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>


void ManageHideStatus(ModelStatus &children, ModelStatus parent)
{
    switch (children)
    {
    case ModelStatus::UNDEFINED :
        children = parent;
        break;
    
    case ModelStatus::SHOW :
        children = parent == ModelStatus::HIDE ? parent : children;
        break;

    case ModelStatus::HIDE :
        children = parent == ModelStatus::SHOW ? parent : children;
        break;

    default: break;
    }
}

void ObjectGroup::update(bool forceUpdate)
{
    bool globalUpdate = state.update();
    globalUpdate |= forceUpdate;

    if(globalUpdate)
    {
        for(auto i = meshes.begin(); i != meshes.end(); i++)
        {
            (*i)->state.modelMatrix = state.modelMatrix * (*i)->state.forceUpdate().modelMatrix;
            ManageHideStatus((*i)->state.hide, state.hide);
        }

        for(auto &i : lights) i->applyModifier(state);

        for(auto i : states)
        {
            i->modelMatrix = state.modelMatrix * i->forceUpdate().modelMatrix;
            ManageHideStatus(i->hide, state.hide);
        }

    }
    
    for(auto &i : children)
    {
        if(i->state.needUpdate() || globalUpdate)
            i->state.modelMatrix = state.modelMatrix * i->state.forceUpdate().modelMatrix;

        ManageHideStatus(i->state.hide, state.hide);
        i->update(true);
    }

}

void ObjectGroup::add(ModelRef meshe)
{
    meshes.push_back(meshe);
}

void ObjectGroup::add(SceneLight light)
{
    // lights.push_back(newLight(*light));
    // lightsDummys.push_back(light);

    lights.push_back(light);
}

void ObjectGroup::add(ObjectGroupRef group)
{
    children.push_back(group);
}

void ObjectGroup::add(ModelStateRef state)
{
    states.push_back(state);
}

void ObjectGroup::setAnimation(SkeletonAnimationState *animation)
{
    for(auto &i : meshes)
        if(i->getVao().animated)
            i->animation = animation;

    for(auto &i : children)
        i->setAnimation(animation);
}

void ObjectGroup::remove(ModelRef mesh)
{
    for(auto i = meshes.begin(); i < meshes.end(); i++)
        if(i->get() == mesh.get())
        {
            meshes.erase(i);
            return;
        }
}

void ObjectGroup::remove(ObjectGroupRef group)
{
    for(auto i = children.begin(); i < children.end(); i++)
        if(i->get() == group.get())
        {
            children.erase(i);
            return;
        }
}

ObjectGroupRef ObjectGroup::copy()
{
    ObjectGroupRef g = newObjectGroup();

    g->name = name;

    g->children.resize(children.size());
    g->states.resize(states.size());
    g->meshes.resize(meshes.size());
    g->lights.resize(lights.size());
    g->state = state.forceUpdate();

    int i = 0;
    for(auto &c : children) g->children[i++] = c->copy();
    i = 0;
    for(auto &c : states) g->states[i++] = ModelStateRef(new ModelState3D(*c));
    i = 0;
    for(auto &c : lights)
        switch (c->getInfos()._infos.a)
        {
            case TUBE_LIGHT : g->lights[i++] = SceneTubeLight(new TubeLight(dynamic_cast<TubeLight&>(*c))); break;
            case POINT_LIGHT : g->lights[i++] = ScenePointLight(new PointLight(dynamic_cast<PointLight&>(*c))); break;
            case DIRECTIONAL_LIGHT : g->lights[i++] = SceneDirectionalLight(new DirectionLight(dynamic_cast<DirectionLight&>(*c))); break;
            default:break;
        }
    i = 0;
    for(auto &c : meshes) g->meshes[i++] = c->copy();

    return g;
}

std::pair<vec3, vec3> ObjectGroup::getMeshesBoundingBox()
{
    vec3 minb(1e6), maxb(-1e6);

    for(auto i : meshes)
    {
        vec4 min1 =  i->state.modelMatrix * vec4(i->getVao()->getAABBMin(), 1.0);
        vec4 max1 =  i->state.modelMatrix * vec4(i->getVao()->getAABBMax(), 1.0);

        minb = min(minb, vec3(min1));
        maxb = max(maxb, vec3(max1));
    }

    for(auto i : children)
    {
        auto m = i->getMeshesBoundingBox();
        minb = min(minb, m.first);
        maxb = max(maxb, m.second);
    }

    return {minb, maxb};
}

ObjectGroup::meshesToBeMerged::meshesToBeMerged(ModelRef m)
{
    material = m->getMaterial();

    uniforms = m->uniforms;
    baseUniforms = m->baseUniforms;

    maps = m->getMaps();
    mapsLocation = m->getMapsLocation();

    models.push_back(m);
}

bool ObjectGroup::meshesToBeMerged::isCompatible(ModelRef m)
{
    if(m->getMaterial() != material)
        return false;

    if(m->getVao().animated)
        return false;

    /*
        TODO : test
    */
    // if(typeid(*m) != typeid(MeshModel3D))
    //     return false;

    int cnt;

    cnt = 0;
    for(auto &l : mapsLocation)
        if(!(m->getMapsLocation()[cnt++] == l))
            return false;

    cnt = 0;
    for(auto &t : maps)
        if(!(m->getMaps()[cnt++].operator==(t)))
            return false;

    cnt = 0;
    for(auto &u : baseUniforms.uniforms)
        if(!(m->baseUniforms.uniforms[cnt++].operator==(u)))
            return false;

    cnt = 0;
    for(auto &u : uniforms.uniforms)
        if(!(m->uniforms.uniforms[cnt++].operator==(u)))
            return false;
    
    return true;
}

void ObjectGroup::populateMeshesToBeMerged(std::vector<meshesToBeMerged> &mtbm)
{
    // std::deque<ModelRef> newMeshes;

    for(auto m : meshes)
    {
        bool isCompatible = false;
        
        for(auto &i : mtbm)
        {
            if(i.isCompatible(m))
            {
                i.models.push_back(m);
                isCompatible = true;
                break;       
            }
        }

        if(!isCompatible)
            mtbm.push_back(meshesToBeMerged(m));
    }

    for(auto c : children)
    {
        c->populateMeshesToBeMerged(mtbm);
    }
}


ObjectGroupRef ObjectGroup::optimizedBatchedCopy()
{
    ObjectGroupRef output(new ObjectGroup());

    output->name   = name + " - Batched Optimized";
    output->state  = state;
    output->states = states;
    output->lights = lights;




    std::vector<meshesToBeMerged> mtbm;

    /****** Creating meshes to be merged ******/
    update(true);
    populateMeshesToBeMerged(mtbm);


    // for(auto &m : mtbm)
    // {
    //     std::cout << "==== Compatible Merge Group ====\n";

    //     for(auto i : m.models)
    //     {
    //         std::cout << "Model " << i->getVao()->getHandle() << "\n";
    //     }
    // }

    mat4 invModel = inverse(state.modelMatrix);

    for(auto &compatiblesMeshes : mtbm)
    {
        int size = compatiblesMeshes.models[0]->getVao()->attributes.size();

        bool usePacking = false;
        bool use3Dpos = false;

        if(size > 0)
        {
            auto &attr = compatiblesMeshes.models[0]->getVao()->attributes[0];
            usePacking = 
                attr.getPerVertexSize() == 4 && 
                (attr.getType() == GL_INT || attr.getType() == GL_UNSIGNED_INT);
        }

        if(size > 2)
        {
            auto &attr = compatiblesMeshes.models[0]->getVao()->attributes[0];
            auto &attrNor = compatiblesMeshes.models[0]->getVao()->attributes[1];
            auto &attrUV = compatiblesMeshes.models[0]->getVao()->attributes[2];

            use3Dpos = 
                attr.getPerVertexSize() == 3 && attr.getType() == GL_FLOAT && 
                attrNor.getPerVertexSize() == 3 && attrNor.getType() == GL_FLOAT &&
                attrUV.getPerVertexSize() == 2 && attrUV.getType() == GL_FLOAT;
        }


        if(compatiblesMeshes.models.size() == 1 || (!usePacking && !use3Dpos) 
        // || !usePacking
        )
        {
            for(auto m : compatiblesMeshes.models)
                output->add(m);

            continue;
        }

        int finalVertexCount = 0;
        int finalFacesCount = 0;

        for(auto m : compatiblesMeshes.models)
        {
            finalVertexCount += m->getVao()->attributes[0].getVertexCount();
            finalFacesCount  += m->getVao().nbFaces;
        }

        GenericSharedBuffer buff1(new char[(usePacking ? sizeof(ivec4) : sizeof(vec3))*(finalVertexCount)]);

        GenericSharedBuffer buff2, buff3;

        if(use3Dpos)
        {
            buff2 = GenericSharedBuffer(new char[sizeof(vec3)*(finalVertexCount)]);
            buff3 = GenericSharedBuffer(new char[sizeof(vec2)*(finalVertexCount)]);
        }

        GenericSharedBuffer nfaces(new char[sizeof(ivec3)*(finalFacesCount/3)]);

        int vid = 0;
        int fid = 0;

        /*
            TODO : add AABB gestion
        */
        for(auto m : compatiblesMeshes.models)
        {
            mat4 transform = invModel * m->state.modelMatrix;

            auto *a = m->getVao()->attributes[0].getBufferAddr();
            auto *b = use3Dpos ? m->getVao()->attributes[1].getBufferAddr() : nullptr;
            auto *c = use3Dpos ? m->getVao()->attributes[2].getBufferAddr() : nullptr;

            int vcount = m->getVao()->attributes[0].getVertexCount();

            /*
                TODO : add normals
            */
            if(usePacking)
                for(int i = 0; i < vcount; i++, vid++)
                {
                    ivec4 v = ((ivec4*)a)[i];

                    vec3 modelPosition = vec3(ivec3(ivec3(v) & (0x00FFFFFF)) - 0x800000)*1e-3f;
                    modelPosition = vec3(transform * vec4(modelPosition, 1));
                    ivec3 packedPosition = (ivec3(modelPosition*1e3f) + 0x800000) & (0x00FFFFFF);

                    v.x &= ~(0x00FFFFFF);
                    v.x |= packedPosition.x;

                    v.y &= ~(0x00FFFFFF);
                    v.y |= packedPosition.y;

                    v.z &= ~(0x00FFFFFF);
                    v.z |= packedPosition.z;

                    ((ivec4*)buff1.get())[vid] = v;
                }
            else
                for(int i = 0; i < vcount; i++, vid++)
                {
                    vec3 v = ((vec3*)a)[i];
                    v = vec3(transform * vec4(v, 1));
                    ((vec3*)buff1.get())[vid] = v;

                    vec3 n = ((vec3*)b)[i];
                    n = vec3(transform * vec4(n, 0));
                    ((vec3*)buff2.get())[vid] = n;

                    ((vec2*)buff3.get())[vid] = ((vec2*)c)[i];
                }
            
            int fcount = m->getVao().nbFaces;

            for(int i = 0; i < fcount/3; i++, fid++)
            {
                ((ivec3*)nfaces.get())[fid] = ((ivec3*)m->getVao().faces.get())[i] + (vid - vcount);
            }
        }

        MeshVao batch;

        if(usePacking)
        {
            batch = MeshVao(new VertexAttributeGroup({
                VertexAttribute(buff1, 0, finalVertexCount, 4, GL_UNSIGNED_INT, false)
            }));
        }
        else
        {
            batch = MeshVao(new VertexAttributeGroup({
                VertexAttribute(buff1, 0, finalVertexCount, 3, GL_FLOAT, false),
                VertexAttribute(buff2, 1, finalVertexCount, 3, GL_FLOAT, false),
                VertexAttribute(buff3, 2, finalVertexCount, 2, GL_FLOAT, false)
            }));
        }

        batch.nbFaces = finalFacesCount;
        batch.faces = nfaces;

        auto m = compatiblesMeshes.models[0]->copy();
        m->setVao(batch);

        m->state.setPosition(vec3(0)).scaleScalar(1);

        if(m->state.useQuaternion)
            m->state.setQuaternion(quat());
        else    
            m->state.setRotation(vec3(0));

        output->add(m);
    }

    return output;
}   
