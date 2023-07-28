#ifndef MESH_HPP
#define MESH_HPP

#include <Shader.hpp>
#include <VertexBuffer.hpp>

#define MESH_BASE_ATTRIBUTE_LOCATION_POSITION 0
#define MESH_BASE_ATTRIBUTE_LOCATION_NORMAL   1
#define MESH_BASE_ATTRIBUTE_LOCATION_COLOR    2

class Mesh
{
    public :
    
        std::shared_ptr<ShaderProgram> material;
        std::shared_ptr<VertexAttributeGroup> vao;

        Mesh(){};
        Mesh(std::shared_ptr<ShaderProgram> material, 
             std::shared_ptr<VertexAttributeGroup> vao)
        : material(material), vao(vao) {};

        void draw(GLenum mode);
        void drawVAO(GLenum mode);
};

#endif