#ifndef MESH_HPP
#define MESH_HPP

#include <string>
#include <Shader.hpp>
#include <VertexBuffer.hpp>
#include <Textures.hpp>

#define MESH_BASE_ATTRIBUTE_LOCATION_POSITION 0
#define MESH_BASE_ATTRIBUTE_LOCATION_COLOR    1
#define MESH_BASE_ATTRIBUTE_LOCATION_NORMAL   2

#define MESH_BASE_UNIFORM_LOCATION_MODEL_MATRIX 3

// typedef std::shared_ptr<ShaderProgram> MeshMaterial;

class MeshMaterial : public std::shared_ptr<ShaderProgram>
{
    public : 
        MeshMaterial(ShaderProgram* material = NULL, ShaderProgram* depthOnlyMaterial = NULL);
        std::shared_ptr<ShaderProgram> depthOnly;
};


typedef std::shared_ptr<VertexAttributeGroup> MeshVao;

class Mesh
{
    protected :
        MeshMaterial material;
        MeshVao vao;
        std::vector<Texture2D> maps;
        std::vector<int> mapsLocation;

    public :

        GLenum defaultMode = GL_TRIANGLES;
        bool invertFaces = false;
        bool depthWrite = true;

        Mesh(){};
        // Mesh(Mesh& mesh);
        Mesh(MeshMaterial material, 
             MeshVao vao)
        : material(material), vao(vao) 
        {
            vao->generate();
        };

        MeshMaterial getMaterial(){return material;};
        MeshVao getVao(){return vao;};

        Mesh& setMaterial(MeshMaterial _material);
        Mesh& setVao(MeshVao _vao);
        Mesh& setMap(Texture2D texture, int location);
        Mesh& removeMap(int location);

        virtual void draw(GLenum mode = 0);
        virtual void drawVAO(GLenum mode = 0);
};

#define MODEL_UNIFORM_START           16
#define MESH_MODEL_UNIFORM_MATRIX     MODEL_UNIFORM_START + 0
#define MESH_MODEL_UNIFORM_ROTATION   MODEL_UNIFORM_START + 1
#define MESH_MODEL_UNIFORM_SCALE      MODEL_UNIFORM_START + 2
#define MESH_MODEL_UNIFORM_POSITION   MODEL_UNIFORM_START + 3

#define ModelRef std::shared_ptr<MeshModel3D> 
#define newModel std::make_shared<MeshModel3D>

/*
    A special type of Mesh that contains additionnal 
    uniforms who will be updated at each drawcall 
    even without material activation.

    Those uniforms are automaticly used to update the
    object's position, rotation & scale.
*/
class MeshModel3D : public Mesh
{
    protected :
        void createUniforms();

    public :
        MeshModel3D() : Mesh()
            {createUniforms();};

        MeshModel3D(Mesh& mesh) : Mesh(mesh)
            {createUniforms();};

        MeshModel3D(MeshMaterial material, MeshVao vao)
            : Mesh(material, vao)
            {createUniforms();};

        MeshModel3D(MeshMaterial material, MeshVao vao, ModelState3D state)
            : Mesh(material, vao), state(state)
            {createUniforms();};


        ModelRef copyWithSharedMesh();

        ModelState3D state;
        ShaderUniformGroup uniforms;

        MeshModel3D& loadFromFolder(
            const std::string folderPath, 
            bool loadColorMap = true, 
            bool loadMaterialMap = true);

        virtual void drawVAO(GLenum mode = GL_TRIANGLES);
        virtual void preDrawRoutine();
};

Mesh readSTL(const std::string filePath);

MeshVao readOBJ(const std::string filePath, bool useVertexColors = false);

#endif