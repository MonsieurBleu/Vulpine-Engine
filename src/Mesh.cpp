#include <Mesh.hpp>

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
