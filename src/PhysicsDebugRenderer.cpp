#include <Utils.hpp>
#include <JoltIntegration/PhysicsDebugRenderer.hpp>
#include <AssetManager.hpp>
#include <Globals.hpp>

#include <cstring>

#define VARIABLE_PRINT(x) #x, ":", x, "\n\t"

#define MAX_INSTANCE_ALLOCATE 256

JoltVulpine::DebugRenderer *JoltVulpine::debugRenderer = nullptr;
JPH::BodyManager::DrawSettings JoltVulpine::debugRendererSettings;
bool JoltVulpine::debugRendererActive = false;

JoltVulpine::DebugRenderer::DebugRenderer()
{
    Initialize();
}

void JoltVulpine::DebugRenderer::DrawLine
(
    JPH::RVec3Arg inFrom,
    JPH::RVec3Arg inTo,
    JPH::ColorArg inColor
)
{
    NOTIF_MESSAGE(
        VARIABLE_PRINT(Jvec3(inFrom)),
        VARIABLE_PRINT(Jvec3(inTo)),
        VARIABLE_PRINT(Jvec3(inColor.ToVec4()))
    )
}


void JoltVulpine::DebugRenderer::DrawTriangle
(
    JPH::RVec3Arg inV1,
    JPH::RVec3Arg inV2,
    JPH::RVec3Arg inV3,
    JPH::ColorArg inColor,
    ECastShadow inCastShadow
)
{
    
}

int meshCount = 0;

JPH::DebugRenderer::Batch JoltVulpine::DebugRenderer::CreateTriangleBatch
(
    const Vertex *inVertices, 
    int inVertexCount, 
    const uint32 *inIndices, 
    int inIndexCount
) 
{
    auto *geo = new JoltVulpine::DebugRenderer::Geometry();

    // NOTIF_MESSAGE(meshCount++)
    // return geo;

    geo->model = newInstancedModel(true);
    geo->modelWire = newInstancedModel(true);

    if(currentThreadID)
    {
        WARNING_MESSAGE("Can't create debug Jolt geometry from threadID ", currentThreadID)
        return geo;
    }

    const uint meshSize = inIndexCount;

    GenericSharedBuffer buff1(new char[sizeof(vec3)*meshSize]);
    GenericSharedBuffer buff2(new char[sizeof(vec3)*meshSize]);
    GenericSharedBuffer buff3(new char[sizeof(vec3)*meshSize]);

    vec3 *pos = (vec3*)buff1.get();
    vec3 *nor = (vec3*)buff2.get();
    vec3 *col = (vec3*)buff3.get();

    for(int i = 0; i < meshSize; i++)
    {
        pos[i] = vec3(Jvec3(inVertices[inIndices[i]].mPosition));
        nor[i] = vec3(Jvec3(inVertices[inIndices[i]].mNormal));
        col[i] = vec3(Jvec3(inVertices[inIndices[i]].mColor.ToVec4()));
    }

    MeshVao vao(new VertexAttributeGroup({
        VertexAttribute(buff1, 0, meshSize, 3, GL_FLOAT, false),
        VertexAttribute(buff2, 1, meshSize, 3, GL_FLOAT, false),
        VertexAttribute(buff3, 2, meshSize, 3, GL_FLOAT, false)
    }));

    geo->model->defaultMode = GL_TRIANGLES;
    geo->model->noBackFaceCulling = true;
    geo->model->setMaterial(Loader<MeshMaterial>::get("basicHelperInstanced"));
    geo->model->setVao(vao);
    geo->model->allocate(MAX_INSTANCE_ALLOCATE);

    globals.getScene()->add(geo->model);
    models.push_back(geo->model);


    MeshVao vao2(new VertexAttributeGroup({
        VertexAttribute(buff1, 0, meshSize, 3, GL_FLOAT, false),
        VertexAttribute(buff2, 1, meshSize, 3, GL_FLOAT, false),
        VertexAttribute(buff3, 2, meshSize, 3, GL_FLOAT, false)
    }));

    geo->modelWire->defaultMode = GL_TRIANGLES;
    geo->modelWire->noBackFaceCulling = true;
    geo->modelWire->setMaterial(Loader<MeshMaterial>::get("basicHelperInstanced2"));
    geo->modelWire->setVao(vao2);
    geo->modelWire->allocate(MAX_INSTANCE_ALLOCATE);

    globals.getScene()->add(geo->modelWire);
    models.push_back(geo->modelWire);

    return geo;
}

JPH::DebugRenderer::Batch JoltVulpine::DebugRenderer::CreateTriangleBatch
(
    const Triangle *inTriangles, 
    int inTriangleCount
) 
{
    auto *geo = new JoltVulpine::DebugRenderer::Geometry();

    // NOTIF_MESSAGE(meshCount++)
    // return geo;

    geo->model = newInstancedModel(true);
    geo->modelWire = newInstancedModel(true);

    if(currentThreadID)
    {
        WARNING_MESSAGE("Can't create debug Jolt geometry from threadID ", currentThreadID)
        return geo;
    }

    const uint meshSize = inTriangleCount*3;

    GenericSharedBuffer buff1(new char[sizeof(vec3)*meshSize]);
    GenericSharedBuffer buff2(new char[sizeof(vec3)*meshSize]);
    GenericSharedBuffer buff3(new char[sizeof(vec3)*meshSize]);

    vec3 *pos = (vec3*)buff1.get();
    vec3 *nor = (vec3*)buff2.get();
    vec3 *col = (vec3*)buff3.get();

    for(int i = 0; i < meshSize; i+=3)
    {
        pos[i] = vec3(Jvec3(inTriangles[i/3].mV[0].mPosition));
        nor[i] = vec3(Jvec3(inTriangles[i/3].mV[0].mNormal));
        col[i] = vec3(Jvec3(inTriangles[i/3].mV[0].mColor.ToVec4()));

        pos[i+1] = vec3(Jvec3(inTriangles[i/3].mV[1].mPosition));
        nor[i+1] = vec3(Jvec3(inTriangles[i/3].mV[1].mNormal));
        col[i+1] = vec3(Jvec3(inTriangles[i/3].mV[1].mColor.ToVec4()));

        pos[i+2] = vec3(Jvec3(inTriangles[i/3].mV[2].mPosition));
        nor[i+2] = vec3(Jvec3(inTriangles[i/3].mV[2].mNormal));
        col[i+2] = vec3(Jvec3(inTriangles[i/3].mV[2].mColor.ToVec4()));
    }

    MeshVao vao(new VertexAttributeGroup({
        VertexAttribute(buff1, 0, meshSize, 3, GL_FLOAT, false),
        VertexAttribute(buff2, 1, meshSize, 3, GL_FLOAT, false),
        VertexAttribute(buff3, 2, meshSize, 3, GL_FLOAT, false)
    }));

    geo->model->defaultMode = GL_TRIANGLES;
    geo->model->noBackFaceCulling = true;
    geo->model->setMaterial(Loader<MeshMaterial>::get("basicHelperInstanced"));
    geo->model->setVao(vao);
    geo->model->allocate(MAX_INSTANCE_ALLOCATE);

    globals.getScene()->add(geo->model);
    models.push_back(geo->model);


    MeshVao vao2(new VertexAttributeGroup({
        VertexAttribute(buff1, 0, meshSize, 3, GL_FLOAT, false),
        VertexAttribute(buff2, 1, meshSize, 3, GL_FLOAT, false),
        VertexAttribute(buff3, 2, meshSize, 3, GL_FLOAT, false)
    }));

    geo->modelWire->defaultMode = GL_TRIANGLES;
    geo->modelWire->noBackFaceCulling = true;
    geo->modelWire->setMaterial(Loader<MeshMaterial>::get("basicHelperInstanced2"));
    geo->modelWire->setVao(vao2);
    geo->modelWire->allocate(MAX_INSTANCE_ALLOCATE);

    globals.getScene()->add(geo->modelWire);
    models.push_back(geo->modelWire);
    
    return geo;
}


void JoltVulpine::DebugRenderer::DrawGeometry
(
    JPH::RMat44Arg inModelMatrix, 
    const JPH::AABox &inWorldSpaceBounds, 
    float inLODScaleSq, 
    JPH::ColorArg inModelColor, 
    const GeometryRef &inGeometry, 
    ECullMode inCullMode, 
    ECastShadow inCastShadow, 
    EDrawMode inDrawMode
)
{
	const LOD *lod = inGeometry->mLODs.data();

    if(globals.currentCamera)
	    lod = &inGeometry->GetLOD(Vvec3(globals.currentCamera->getPosition()), inWorldSpaceBounds, inLODScaleSq);


    auto *geo = (JoltVulpine::DebugRenderer::Geometry*)lod->mTriangleBatch.GetPtr();
    
    ModelInstance *i = nullptr;

    switch (inDrawMode)
    {
        case EDrawMode::Wireframe : i = geo->modelWire->createInstance(); break;
        default : i = geo->model->createInstance(); break;
    }

    if(!i) return;

    i->modelMatrix = *(mat4*)(&inModelMatrix);
    i->userData = uvec4(inModelColor.r, inModelColor.g, inModelColor.b, 0);
}


void JoltVulpine::DebugRenderer::DrawText3D
(
        JPH::RVec3Arg inPosition,
        const std::string_view & inString,
        JPH::ColorArg inColor,
        float inHeight
)
{
    // NOTIF_MESSAGE(
    //     VARIABLE_PRINT(Jvec3(inPosition)),
    //     VARIABLE_PRINT(Jvec3(inColor.ToVec4())),
    //     VARIABLE_PRINT(inHeight)
    //     VARIABLE_PRINT(inString)
    // )
}