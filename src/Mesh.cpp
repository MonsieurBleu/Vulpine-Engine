#include <Mesh.hpp>

#include <iostream>
#include <fstream>

#include <Utils.hpp>

using namespace glm;

// Mesh::Mesh(Mesh& mesh)
// {
//     material = mesh.material;
//     vao = mesh.vao;
// }

void Mesh::drawVAO(GLenum mode)
{
    glBindVertexArray(vao->getHandle());
    glDrawArrays(mode, 0, vao->attributes[MESH_BASE_ATTRIBUTE_LOCATION_POSITION].getVertexCount());
}

void Mesh::draw(GLenum mode) 
{
    material->activate();
    drawVAO(mode);
    material->deactivate();
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

Mesh& Mesh::setColorMap(Texture2D texture)
{
    colorMap = texture;
    return *this;
}

void MeshModel3D::preDrawRoutine(){}

void MeshModel3D::drawVAO(GLenum mode)
{
    preDrawRoutine();

    state.update();
    uniforms.update();

    colorMap.bind(0);

    if(invertFaces)
        glCullFace(GL_FRONT);
    if(!depthWrite)
        glDisable(GL_DEPTH_TEST);


    glBindVertexArray(vao->getHandle());
    glDrawArrays(mode, 0, vao->attributes[MESH_BASE_ATTRIBUTE_LOCATION_POSITION].getVertexCount());

    if(invertFaces)
        glCullFace(GL_BACK);
    if(!depthWrite)
        glEnable(GL_DEPTH_TEST);
}

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

MeshVao readOBJ(const std::string filePath, bool useVertexColors)
{
    FILE *obj = fopen(filePath.c_str(), "r");

    fseek(obj, 0, SEEK_END);
    const uint64 fsize = ftell(obj);
    fseek(obj, 0, SEEK_SET);

    if(obj == nullptr || fsize == UINT64_MAX)
    {
        std::cout << TERMINAL_ERROR;
        perror("readOBJ : ");
        std::cout << "\n The loader will return an empty object.\n" << TERMINAL_RESET;

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