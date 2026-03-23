#include <Graphics/Query.hpp>

#include <AssetManager.hpp>

#include <glm/glm.hpp>
using namespace glm;

// OcclussionQuery::OcclussionQuery(vec3 middle, vec3 extents, bool * active) 
// : 
//     // MeshModel3D(
//     //     Loader<MeshMaterial>::get("depthQuerry"),
//     //     Loader<MeshVao>::get("QueryBox") /* TODO : add*/
//     // ),
//     middle(middle), extents(extents), active(active)
// {
//     glGenQueries(1, &id);
// }

OcclussionQuery::OcclussionQuery()
{
    glGenQueries(1, (GLuint *)&id);

}

OcclussionQuery::~OcclussionQuery()
{
    // if(id >= 0) glDeleteQueries(1, (GLuint *)&id);
}

void OcclussionQuery::startQuery()
{
    // if(id < 0) glGenQueries(1, (GLuint *)&id);

    glBeginQuery(GL_SAMPLES_PASSED, id);
}

void OcclussionQuery::stopQuery()
{
    glEndQuery(GL_SAMPLES_PASSED);
}

void OcclussionQuery::retreiveQueryResults()
{
    // glGetQueryObjectiv(id, GL_QUERY_RESULT_NO_WAIT, (GLint*)&occluded);
    // glGetQueryObjectuiv(id, GL_QUERY_RESULT, &occluded);
    glGetQueryObjectiv(id, GL_QUERY_RESULT, (GLint*)&occluded);

    // WARNING_MESSAGE(id, "\t\t", occluded);
}

bool OcclussionQuery::isQueryActive(int layer)
{
    if(active) return active[layer];
    else return true;
}

GLint OcclussionQuery::getQueryResult()
{
    // ERROR_MESSAGE(id, "\t\t", occluded);
    return occluded + containCamera;
}