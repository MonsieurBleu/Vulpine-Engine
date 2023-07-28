#include <Mesh.hpp>

void Mesh::drawVAO(GLenum mode = GL_TRIANGLES)
{
    glBindVertexArray(vao->getHandle());
    glDrawArrays(mode, 0, vao->attributes[MESH_BASE_ATTRIBUTE_LOCATION_POSITION].getVertexCount());

}

void Mesh::draw(GLenum mode = GL_TRIANGLES) 
{
    material->activate();

    drawVAO(mode);

    material->deactivate();
}
