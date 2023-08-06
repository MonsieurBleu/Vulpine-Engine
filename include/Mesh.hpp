#ifndef MESH_HPP
#define MESH_HPP

#include <string>
#include <Shader.hpp>
#include <VertexBuffer.hpp>

#define MESH_BASE_ATTRIBUTE_LOCATION_POSITION 0
#define MESH_BASE_ATTRIBUTE_LOCATION_COLOR    1
#define MESH_BASE_ATTRIBUTE_LOCATION_NORMAL   2

#define MESH_BASE_UNIFORM_LOCATION_MODEL_MATRIX 3

typedef std::shared_ptr<ShaderProgram> MeshMaterial;
typedef std::shared_ptr<VertexAttributeGroup> MeshVao;

class Mesh
{
    protected :
        MeshMaterial material;
        MeshVao vao;

    public :
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

        virtual void draw(GLenum mode = GL_TRIANGLES);
        virtual void drawVAO(GLenum mode = GL_TRIANGLES);
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
    private :

        ShaderUniformGroup uniforms;

        void createUniforms();

    public :
        MeshModel3D() : Mesh(){};

        MeshModel3D(Mesh& mesh) : Mesh(mesh)
            {createUniforms();};

        MeshModel3D(MeshMaterial material, MeshVao vao)
            : Mesh(material, vao)
            {createUniforms();};

        MeshModel3D(MeshMaterial material, MeshVao vao, ModelState3D state)
            : Mesh(material, vao), state(state)
            {createUniforms();};

        ModelState3D state;

        void drawVAO(GLenum mode = GL_TRIANGLES);
};

Mesh readSTL(const std::string filePath);

MeshVao readOBJ(const std::string filePath);

#endif