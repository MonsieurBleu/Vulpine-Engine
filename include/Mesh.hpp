#ifndef MESH_HPP
#define MESH_HPP

#include <Shader.hpp>
#include <VertexBuffer.hpp>

#define MESH_BASE_ATTRIBUTE_LOCATION_POSITION 0
#define MESH_BASE_ATTRIBUTE_LOCATION_COLOR    1
#define MESH_BASE_ATTRIBUTE_LOCATION_NORMAL   2

typedef std::shared_ptr<ShaderProgram> MeshMaterial;
typedef std::shared_ptr<VertexAttributeGroup> MeshVao;

class Mesh
{
    public :

        MeshMaterial material;
        MeshVao vao;

        Mesh(){};
        Mesh(MeshMaterial material, 
             MeshVao vao)
        : material(material), vao(vao) 
        {
            vao->generate();
        };

        void draw(GLenum mode = GL_TRIANGLES);
        void drawVAO(GLenum mode = GL_TRIANGLES);
};

#endif