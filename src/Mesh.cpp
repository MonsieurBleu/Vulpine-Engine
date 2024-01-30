#include <Mesh.hpp>

#include <iostream>
#include <fstream>

#include <Utils.hpp>

#include <Globals.hpp>

using namespace glm;

// Mesh::Mesh(Mesh& mesh)
// {
//     material = mesh.material;
//     vao = mesh.vao;
// }

MeshMaterial::MeshMaterial(ShaderProgram* material, ShaderProgram* depthOnlyMaterial)
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

GLuint Mesh::drawVAO(GLenum mode)
{
    glBindVertexArray(vao->getHandle());
    // if(!mode)
        mode = defaultMode;

    glDrawArrays(mode, 0, vao->attributes[MESH_BASE_ATTRIBUTE_LOCATION_POSITION].getVertexCount());

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


Mesh& Mesh::setMaterial(MeshMaterial _material)
{
    material = _material;
    return *this;
}

Mesh& Mesh::setVao(MeshVao _vao)
{
    vao = _vao;
    vao->generate();
    return *this;
}

Mesh& Mesh::setMap(Texture2D texture, int location)
{
    maps.push_back(texture);
    mapsLocation.push_back(location);
    return (*this);
}

Mesh& Mesh::removeMap(int location)
{
    int size = maps.size();
    std::vector<Texture2D> newMaps;
    std::vector<int> newMapsLocation;
    for(auto i = 0; i < size; i++)
        if(i != location)
        {
            newMaps.push_back(maps[i]);
            newMapsLocation.push_back(mapsLocation[i]);
        }

    maps = newMaps;
    mapsLocation = newMapsLocation;

    return *this;
}

bool Mesh::cull(){return true;};

bool MeshModel3D::cull()
{
    if(state.hide == ModelStateHideStatus::HIDE)
        return culled = false;
    
    if(!state.frustumCulled)
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
        length(vec3(m[2]))
    );
    float radius = max(length(vao->getAABBMax()*scale), length(vao->getAABBMin()*scale));
    const Frustum f = globals.currentCamera->getFrustum();   

    return culled =                   
        dot(f.left.normal  , center-f.left.position  ) > -radius &&
        dot(f.right.normal , center-f.right.position ) > -radius &&
        dot(f.far.normal   , center-f.far.position   ) > -radius &&
        dot(f.near.normal  , center-f.near.position  ) > -radius &&
        dot(f.top.normal   , center-f.top.position   ) > -radius &&
        dot(f.bottom.normal, center-f.bottom.position) > -radius;
}

void MeshModel3D::update()
{
    state.update(); // can be removed if the scene arleady do the update
    uniforms.update();
}

void Mesh::bindAllMaps()
{
    for(size_t i = 0; i < maps.size(); i ++)
        if(maps[i].getHandle())
            maps[i].bind(mapsLocation[i]);
}

void Mesh::setBindlessMaps()
{
    for(size_t i = 0; i < maps.size(); i ++)
        if(maps[i].getBindlessHandle())
            glUniform1ui64ARB(MESH_BASE_UNIFORM_LOCATION_MAP+i, maps[i].getBindlessHandle());
}

void MeshModel3D::setDrawMode()
{
    if(invertFaces)
        glCullFace(GL_FRONT);
    if(!depthWrite)
        glDisable(GL_DEPTH_TEST);
    if(noBackFaceCulling)
        glDisable(GL_CULL_FACE); 
}

void MeshModel3D::resetDrawMode()
{
    if(invertFaces)
        glCullFace(GL_BACK);
    if(!depthWrite)
        glEnable(GL_DEPTH_TEST);
    if(noBackFaceCulling)
        glEnable(GL_CULL_FACE); 
}

GLuint MeshModel3D::drawVAO(GLenum mode)
{
    if(!culled)
        return 0;

    update();
    setDrawMode();

    glBindVertexArray(vao->getHandle());
    mode = defaultMode;
    glDrawArrays(mode, 0, vao->attributes[MESH_BASE_ATTRIBUTE_LOCATION_POSITION].getVertexCount());
    
    resetDrawMode();

    return 1;
}

bool MeshModel3D::isCulled(){return culled;};

void MeshModel3D::createUniforms()
{
    uniforms = ShaderUniformGroup(
        {
            ShaderUniform(&state.modelMatrix, MESH_MODEL_UNIFORM_MATRIX),
            ShaderUniform(&state.rotationMatrix, MESH_MODEL_UNIFORM_ROTATION),
            ShaderUniform(&state.scale, MESH_MODEL_UNIFORM_SCALE),
            ShaderUniform(&state.position, MESH_MODEL_UNIFORM_POSITION)
        });
}

MeshModel3D& MeshModel3D::loadFromFolder(
    const std::string folderPath, 
    bool loadColorMap, 
    bool loadMaterialMap)
{
    setVao(readOBJ(folderPath + "model.obj"));
    
    if(loadColorMap)
        setMap(
            Texture2D().loadFromFileKTX((folderPath + "CE.ktx").c_str()),
            0);

    if(loadMaterialMap)
        setMap(
            Texture2D().loadFromFileKTX((folderPath + "NRM.ktx").c_str()),
            1);

    return (*this);
}

MeshVao readOBJ(const std::string filePath, bool useVertexColors)
{
    FILE *obj = fopen(filePath.c_str(), "r");

    fseek(obj, 0, SEEK_END);
    const uint64 fsize = ftell(obj);
    fseek(obj, 0, SEEK_SET);

    if(obj == nullptr || fsize == UINT64_MAX)
    {
        std::cerr
        << TERMINAL_ERROR << "Error loading file : "
        << TERMINAL_FILENAME << filePath 
        << TERMINAL_ERROR << "\n";
        perror("\treadOBJ");
        std::cerr << "\tThe loader will return an empty object.\n" << TERMINAL_RESET;

        return MeshVao();
    }

    char *data = new char[fsize]{'\0'};

    fread(data, fsize, 1, obj);
    fclose(obj);

    std::vector<vec3> tempVertices;
    std::vector<vec2> tempUvs;
    std::vector<vec3> tempColors;
    std::vector<vec3> tempNormals;

    const int f_ = 0x2066;
    const int v_ = 0x2076;
    const int vt = 0x7476;
    const int vn = 0x6E76;

    char *reader = data;

    int faceCounter = 0;

    while(reader < data+fsize)
    {
        reader = strchr(reader, '\n');

        if(!reader) break;

        reader ++;

        vec3 buff;
        vec3 buff2;

        switch (((uint16*)reader)[0])
        {
        case vt :
                sscanf(reader+2, "%f %f %f", &buff.x, &buff.y, &buff.z);
                tempUvs.push_back(buff);
            break;

        case v_ :
            if(useVertexColors)
            {
                sscanf(reader+2, "%f %f %f %f %f %f", &buff.x, &buff.y, &buff.z, &buff2.x, &buff2.y, &buff2.z);
                tempVertices.push_back(buff);
                tempColors.push_back(buff2);
            }
            else
            {
                sscanf(reader+2, "%f %f %f", &buff.x, &buff.y, &buff.z);
                tempVertices.push_back(buff);                
            }
            break;

        case vn :
            sscanf(reader+2, "%f %f %f", &buff.x, &buff.y, &buff.z);
            tempNormals.push_back(buff);
            break;

        case f_ :
            faceCounter ++;
            break;

        default:
            break;
        }
    }

    reader = data;
    GenericSharedBuffer positions = GenericSharedBuffer(new char[sizeof(float)*9*faceCounter]);
    char *positionWriter = positions.get();

    GenericSharedBuffer normals = GenericSharedBuffer(new char[sizeof(float)*9*faceCounter]);
    char *normalWriter = normals.get();

    int colorChannelSize = useVertexColors ? 3 : 2;

    GenericSharedBuffer colors = GenericSharedBuffer(new char[sizeof(float)*3*colorChannelSize*faceCounter]);
    char *colorWriter = colors.get();

    unsigned int vertexIndex[3];
    unsigned int uvIndex[3];
    unsigned int normalIndex[3];

    while(reader < data+fsize)
    {
        reader = strchr(reader, '\n');

        if(!reader) break;
        reader ++;
        if(((uint16*)reader)[0] == f_)
        {
            
            
            if(useVertexColors)
            {
                sscanf(reader+2, "%u//%u %u//%u %u//%u\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2] );
                for(int i = 0; i < 3; i++)
                {
                    memcpy(positionWriter, (void*)&tempVertices[vertexIndex[i]-1], sizeof(vec3)); 
                    positionWriter += sizeof(vec3);

                    memcpy(colorWriter, (void*)&tempColors[vertexIndex[i]-1], sizeof(vec3)); 
                    colorWriter += sizeof(vec3);

                    memcpy(normalWriter, (void*)&tempNormals[normalIndex[i]-1], sizeof(vec3)); 
                    normalWriter += sizeof(vec3);
                }
            }
            else
            {
                sscanf(reader+2, "%u/%u/%u %u/%u/%u %u/%u/%u\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
                for(int i = 0; i < 3; i++)
                {
                    memcpy(positionWriter, (void*)&tempVertices[vertexIndex[i]-1], sizeof(vec3)); 
                    positionWriter += sizeof(vec3);

                    memcpy(colorWriter, (void*)&tempUvs[uvIndex[i]-1], sizeof(vec2)); 
                    colorWriter += sizeof(vec2);

                    memcpy(normalWriter, (void*)&tempNormals[normalIndex[i]-1], sizeof(vec3)); 
                    normalWriter += sizeof(vec3);
                }
            }


        }

    }

    delete data;

    MeshVao newVao(new VertexAttributeGroup(
        {
            VertexAttribute(positions, 0, faceCounter*3, 3, GL_FLOAT, false),
            VertexAttribute(normals, 1, faceCounter*3, 3, GL_FLOAT, false),
            VertexAttribute(colors, 2, faceCounter*3, colorChannelSize, GL_FLOAT, false)
        }
    ));
    
    newVao->generate(); 

    return newVao;
}


ModelRef  MeshModel3D::copyWithSharedMesh()
{
    ModelRef m = newModel(material, vao);
    m->maps = maps;
    m->mapsLocation = mapsLocation;
    return m;
}

void Mesh::bindMap(int id, int location)
{
    if(id > (int)maps.size()) return;
    maps[id].bind(location);
}

void InstancedMeshModel3D::allocate(size_t maxInstanceCount)
{
    if(!vao.get()) return;

    allocatedInstance = maxInstanceCount;

    instances = std::make_shared<ModelInstance*>(new ModelInstance[allocatedInstance]);

    matricesBuffer = std::shared_ptr<VertexAttribute>(new VertexAttribute(
        GenericSharedBuffer(new char[allocatedInstance*sizeof(mat4)]),
        3, allocatedInstance, 16, GL_FLOAT, false
        ));
    
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
    const int end = beg+3;
    for(int i = beg; i <= end; i++)
    {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)((i-beg)*sizeof(vec4)));
        glVertexAttribDivisor(i, 1);
    }

    glBindVertexArray(0);
}   

GLuint InstancedMeshModel3D::drawVAO(GLenum mode)
{
    update();
    setDrawMode();

    glBindVertexArray(vao->getHandle());
    glDrawArraysInstanced(
        defaultMode, 
        0,
        vao->attributes[MESH_BASE_ATTRIBUTE_LOCATION_POSITION].getVertexCount(),
        drawnInstance
    );
    
    resetDrawMode();

    return 1;
}

bool InstancedMeshModel3D::cull()
{
    return true;
}

ModelInstance* InstancedMeshModel3D::createInstance()
{
    if(createdInstance >= allocatedInstance)
        return nullptr;

    ModelInstance* newInstance = &(*instances.get())[createdInstance];
    createdInstance++;
    return newInstance;
}

void InstancedMeshModel3D::updateInstances()
{
    mat4 *m = (mat4*)matricesBuffer->getBufferAddr();

    // bool bufferNeedUpdate = false;

    drawnInstance = 0;

    for(size_t i = 0; i < createdInstance; i++)
    {
        ModelInstance &inst = (*instances)[i];

        if(inst.hide != ModelStateHideStatus::HIDE)
        {
            memcpy((void*)&m[i], (void*)&inst.modelMatrix, sizeof(mat4));
            drawnInstance ++;
        }
    }

    // matricesBuffer->sendAllToGPU();

    glBindBuffer(GL_ARRAY_BUFFER, matricesBuffer->getHandle());
    glBufferData(
        GL_ARRAY_BUFFER, 
        sizeof(mat4)*drawnInstance, 
        matricesBuffer->getBufferAddr(), 
        GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}