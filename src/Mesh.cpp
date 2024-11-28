
#include <iostream>
#include <fstream>

#include <Graphics/Mesh.hpp>
#include <Utils.hpp>
#include <Globals.hpp>
#include <VulpineAssets.hpp>
#include <Graphics/Skeleton.hpp>

using namespace glm;

// Mesh::Mesh(Mesh& mesh)
// {
//     material = mesh.material;
//     vao = mesh.vao;
// }

MeshVao::MeshVao(VertexAttributeGroup *ptr)
    : std::shared_ptr<VertexAttributeGroup>(ptr)
{
}

void MeshVao::generateEBO()
{
    if(!nbFaces || eboHandle.get())
        return;

    eboHandle = std::shared_ptr<GLuint>(new GLuint());

    glBindVertexArray(this->get()->getHandle());

    glGenBuffers(1, eboHandle.get());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *eboHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, nbFaces * sizeof(unsigned int), faces.get(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

MeshVao::~MeshVao()
{
    if (eboHandle && *eboHandle && eboHandle.use_count() == 1)
    {
        glDeleteVertexArrays(1, eboHandle.get());
    }
}


MeshMaterial::MeshMaterial(ShaderProgram *material, ShaderProgram *depthOnlyMaterial)
    : std::shared_ptr<ShaderProgram>(material), depthOnly(depthOnlyMaterial)
{
}

MeshMaterial::~MeshMaterial()
{
    // if(use_count() == 1 && get())
    // {
    //     std::cout << "Deleting material " << get()->getProgram() << "\n";
    //     glDeleteProgram(get()->getProgram());
    // }
}

GLuint Mesh::drawVAO(bool depthOnly)
{
    glBindVertexArray(vao->getHandle());

    glDrawArrays(defaultMode, 0, vao->attributes[MESH_BASE_ATTRIBUTE_LOCATION_POSITION].getVertexCount());

    return 1;
}

GLuint Mesh::draw(GLenum mode)
{
    material->activate();
    bindAllMaps();
    GLuint dc = drawVAO(mode);
    material->deactivate();
    return dc;
}

Mesh &Mesh::setMaterial(MeshMaterial _material)
{
    material = _material;
    return *this;
}

Mesh &Mesh::setVao(MeshVao _vao)
{
    vao = _vao;
    vao->generate();
    vao.generateEBO();
    return *this;
}

Mesh &Mesh::setMap(int location, Texture2D texture)
{
    maps.push_back(texture);
    mapsLocation.push_back(location);
    return (*this);
}

Mesh &Mesh::setMap(Texture2D texture, int location)
{
    return setMap(location, texture);
}

Mesh &Mesh::removeMap(int location)
{
    int size = maps.size();
    std::vector<Texture2D> newMaps;
    std::vector<int> newMapsLocation;
    for (auto i = 0; i < size; i++)
        if (i != location)
        {
            newMaps.push_back(maps[i]);
            newMapsLocation.push_back(mapsLocation[i]);
        }

    maps = newMaps;
    mapsLocation = newMapsLocation;

    return *this;
}

bool Mesh::cull() { return true; };

bool MeshModel3D::cull()
{
    if (state.hide == ModelStatus::HIDE)
        return culled = false;

    if (!state.frustumCulled)
        return culled = true;

    // return culled = false;

    const mat4 m = state.modelMatrix;
    vec3 center = vec3(m[3]);

    // vec3 camToCenter = center-globals.currentCamera->getPosition();
    // if(dot(camToCenter, camToCenter) >= 90000.f)
    //     return culled = false;

    vec3 scale = vec3(
        length(vec3(m[0])),
        length(vec3(m[1])),
        length(vec3(m[2])));
    float radius = max(length(vao->getAABBMax() * scale), length(vao->getAABBMin() * scale));
    const Frustum f = globals.currentCamera->getFrustum();

    return culled =
               dot(f.left.normal, center - f.left.position) > -radius &&
               dot(f.right.normal, center - f.right.position) > -radius &&
               dot(f.far.normal, center - f.far.position) > -radius &&
               dot(f.near.normal, center - f.near.position) > -radius &&
               dot(f.top.normal, center - f.top.position) > -radius &&
               dot(f.bottom.normal, center - f.bottom.position) > -radius;
}

void MeshModel3D::update()
{
    state.update(); // can be removed if the scene arleady do the update
    baseUniforms.update();
    if(animation)
        animation->activate(2);
}

void Mesh::bindAllMaps()
{
    for (size_t i = 0; i < maps.size(); i++)
        if (maps[i].getHandle())
            maps[i].bind(mapsLocation[i]);
}

void Mesh::setBindlessMaps()
{
    for (size_t i = 0; i < maps.size(); i++)
        if (maps[i].getBindlessHandle())
            glUniform1ui64ARB(MESH_BASE_UNIFORM_LOCATION_MAP + i, maps[i].getBindlessHandle());
}

void MeshModel3D::setDrawMode()
{
    if (invertFaces)
        glCullFace(GL_FRONT);
    if (!depthWrite)
        glDisable(GL_DEPTH_TEST);
    if (noBackFaceCulling)
        glDisable(GL_CULL_FACE);
}

void MeshModel3D::resetDrawMode()
{
    if (invertFaces)
        glCullFace(GL_BACK);
    if (!depthWrite)
        glEnable(GL_DEPTH_TEST);
    if (noBackFaceCulling)
        glEnable(GL_CULL_FACE);
}

GLuint MeshModel3D::drawVAO(bool depthOnly)
{
    if(!vao.get() || !vao.get()->attributes.size()) return 0;
    
    update();
    if(!depthOnly) uniforms.update();
    setDrawMode();

    glBindVertexArray(vao->getHandle());

    if(vao.faces.get() && vao.eboHandle.get())
    {
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *vao.eboHandle);
        glDrawElements(defaultMode, vao.nbFaces,  GL_UNSIGNED_INT, 0);
    }
    else
        glDrawArrays(defaultMode, 0, vao->attributes[MESH_BASE_ATTRIBUTE_LOCATION_POSITION].getVertexCount());

    resetDrawMode();

    return 1;
}

bool MeshModel3D::isCulled() { return culled; };

void MeshModel3D::createUniforms()
{
    baseUniforms = ShaderUniformGroup(
        {ShaderUniform(&state.modelMatrix, MESH_MODEL_UNIFORM_MATRIX),
        //  ShaderUniform(&state.rotationMatrix, MESH_MODEL_UNIFORM_ROTATION),
        //  ShaderUniform(&state.scale, MESH_MODEL_UNIFORM_SCALE),
        //  ShaderUniform(&state.position, MESH_MODEL_UNIFORM_POSITION)
         });
}

MeshModel3D &MeshModel3D::loadFromFolder(
    const std::string folderPath,
    bool loadColorMap,
    bool loadMaterialMap)
{
    setVao(readOBJ(folderPath + "model.obj"));
    
    if(loadColorMap)
        setMap(
            Texture2D().loadFromFileKTX((folderPath + "CE.ktx").c_str()),
            0);

    if (loadMaterialMap)
        setMap(
            Texture2D().loadFromFileKTX((folderPath + "NRM.ktx").c_str()),
            1);

    return (*this);
}

MeshModel3D &MeshModel3D::loadFromFolderVulpine(
    const std::string folderPath,
    bool loadColorMap,
    bool loadMaterialMap)
{
    setVao(loadVulpineMesh((folderPath + "model.vulpineMesh").c_str()));
    
    if(loadColorMap)
        setMap(
            Texture2D().loadFromFileKTX((folderPath + "CE.ktx").c_str()),
            0);

    if (loadMaterialMap)
        setMap(
            Texture2D().loadFromFileKTX((folderPath + "NRM.ktx").c_str()),
            1);

    return (*this);
}

void MeshModel3D::tessHeightTextureRange(vec2 min, vec2 max)
{
    lodHeigtTextureRange = vec4(min, max);
}

void MeshModel3D::tessHeighFactors(float uvScale, float heightFactor)
{
    lodHeightDispFactors.z = uvScale;
    lodHeightDispFactors.w = heightFactor;
}

void MeshModel3D::tessDisplacementFactors(float uvScale, float displacementFactor)
{
    lodHeightDispFactors.x = uvScale;
    lodHeightDispFactors.y = displacementFactor;
}

void MeshModel3D::tessActivate(vec2 minmaxTessLevel, vec2 minmaxDistance)
{
    lodTessLevelDistance = vec4(minmaxTessLevel, minmaxDistance);
    baseUniforms.add(ShaderUniform(&lodHeigtTextureRange, 10));
    baseUniforms.add(ShaderUniform(&lodHeightDispFactors, 11));
    baseUniforms.add(ShaderUniform(&lodTessLevelDistance, 12));
}

ModelRef MeshModel3D::copy()
{
    ModelRef m = newModel(material, vao);
    m->maps = maps;
    m->mapsLocation = mapsLocation;
    m->invertFaces = invertFaces;
    m->noBackFaceCulling = noBackFaceCulling;
    m->defaultMode = defaultMode;
    m->state.frustumCulled = state.frustumCulled;
    m->depthWrite = depthWrite;
    m->uniforms = uniforms;
    m->state = state;
    return m;
}

void Mesh::bindMap(int id, int location)
{
    if (id > (int)maps.size())
        return;
    maps[id].bind(location);
}

void InstancedMeshModel3D::allocate(size_t maxInstanceCount)
{
    if (!vao.get())
        return;

    allocatedInstance = maxInstanceCount;

    // instances = std::make_shared<ModelInstance *>(new ModelInstance[allocatedInstance]);
    instances.resize(allocatedInstance);

    matricesBuffer = std::shared_ptr<VertexAttribute>(new VertexAttribute(
        GenericSharedBuffer(new char[allocatedInstance * sizeof(mat4)]),
        3, allocatedInstance, 16, GL_FLOAT, false));

    matricesBuffer->genBuffer();
    matricesBuffer->sendAllToGPU();

    // glBindVertexArray(vao->getHandle());
    // glBindVertexBuffer(
    //     3,
    //     matricesBuffer->getHandle(),
    //     0,
    //     sizeof(vec4));

    // glVertexAttribBinding(3, 3);
    // matricesBuffer->setFormat();
    glBindBuffer(GL_ARRAY_BUFFER, matricesBuffer->getHandle());
    glBindVertexArray(vao->getHandle());

    const int beg = MESH_BASE_ATTRIBUTE_LOCATION_INSTANCE;
    const int end = beg + 3;
    for (int i = beg; i <= end; i++)
    {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void *)((i - beg) * sizeof(vec4)));
        glVertexAttribDivisor(i, 1);
    }

    glBindVertexArray(0);
}

GLuint InstancedMeshModel3D::drawVAO(bool depthOnly)
{
    update();
    if(!depthOnly) uniforms.update();
    setDrawMode();

    glBindVertexArray(vao->getHandle());

    if(vao.faces.get() && vao.eboHandle.get())
        glDrawElementsInstanced(defaultMode, vao.nbFaces, GL_UNSIGNED_INT, 0, drawnInstance);
    else
        glDrawArraysInstanced(
            defaultMode,
            0,
            vao->attributes[MESH_BASE_ATTRIBUTE_LOCATION_POSITION].getVertexCount(),
            drawnInstance);

    resetDrawMode();

    return 1;
}

bool InstancedMeshModel3D::cull()
{
    return true;
}

ModelInstance *InstancedMeshModel3D::createInstance()
{
    if (createdInstance >= allocatedInstance)
        return nullptr;

    ModelInstance *newInstance = &instances[createdInstance];
    createdInstance++;
    return newInstance;
}

void InstancedMeshModel3D::updateInstances()
{
    mat4 *m = (mat4 *)matricesBuffer->getBufferAddr();

    // bool bufferNeedUpdate = false;

    drawnInstance = 0;

    for (size_t i = 0; i < createdInstance; i++)
    {
        ModelInstance &inst = instances[i];

        inst.update();

        if (inst.hide != ModelStatus::HIDE)
        {
            // memcpy((void *)&m[i], (void *)&inst.modelMatrix, sizeof(mat4));
            m[i] = inst.modelMatrix;
            drawnInstance++;
        }
    }

    // matricesBuffer->sendAllToGPU();

    glBindBuffer(GL_ARRAY_BUFFER, matricesBuffer->getHandle());
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(mat4) * drawnInstance,
        matricesBuffer->getBufferAddr(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}