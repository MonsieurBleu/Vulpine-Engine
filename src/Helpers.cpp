#include <Helpers.hpp>
#include <Globals.hpp>
#include <iostream>
#include <Globals.hpp>
#include <MathsUtils.hpp>
#include <Constants.hpp>

// PointLightHelper::PointLightHelper(ScenePointLight light) : light(light)
// {
//     MeshMaterial uvPhong(
//         new ShaderProgram(
//             "shader/foward rendering/uv/phong.frag", 
//             "shader/foward rendering/uv/phong.vert", 
//             "", 
//             globals.standartShaderUniform3D() 
//         ));
    
//     ModelRef model = newModel();

//     model
//         ->setVao(readOBJ("ressources/test/jug.obj", false))
//         .setMaterial(uvPhong)
//         .setColorMap(Texture2D().loadFromFile("ressources/test/jug.jpg").generate());
    
//     add(model);
// }

// void PointLightHelper::update(bool forceUpdate)
// {
//     std::cout << "unpog\n";
//     meshes.front()->state.position = light->position();
//     meshes.front()->state.scale = vec3(light->radius());
//     ObjectGroup::update(forceUpdate);
// }

PointLightHelperMODEL::PointLightHelperMODEL(ScenePointLight light) : light(light)
{
    setVao(readOBJ("ressources/helpers/PointLight.obj"));
    setMaterial(MeshMaterial(
        new ShaderProgram(
            "shader/foward rendering/phong.frag", 
            "shader/foward rendering/phong.vert", 
            "", 
            globals.standartShaderUniform3D())));
    state = ModelState3D().scaleScalar(3.0);

    createUniforms();
}

// void PointLightHelper::drawVAO(GLenum mode)
// {
//     // state.setPosition(light->position());
//     // state.scaleScalar(light->radius());
//     // state.forceUpdate();
//     // material->uniforms.update();
//     // std::cout << state.scale.x << "\n";
//     // std::cout << uniforms;
//     // this->MeshModel3D::drawVAO(mode);

//     state.setPosition(vec3(10, 1, 10));
//     state.scaleScalar(5.0);

//     state.update();
//     uniforms.update();

//     colorMap.bind(0);

//     if(invertFaces)
//         glCullFace(GL_FRONT);
//     if(!depthWrite)
//         glDisable(GL_DEPTH_TEST);


//     glBindVertexArray(vao->getHandle());
//     glDrawArrays(mode, 0, vao->attributes[MESH_BASE_ATTRIBUTE_LOCATION_POSITION].getVertexCount());

//     if(invertFaces)
//         glCullFace(GL_BACK);
//     if(!depthWrite)
//         glEnable(GL_DEPTH_TEST);
// }

// void PointLightHelper::draw(GLenum mode)
// {
//     material->activate();
//     PointLightHelper::drawVAO();
//     material->deactivate();
// }


PointLightHelper::PointLightHelper(ScenePointLight light) : light(light)
{
    // static MeshMaterial mat(new ShaderProgram(
    //             "shader/foward rendering/basic.frag", 
    //             "shader/foward rendering/basic.vert", 
    //             "", 
    //             globals.standartShaderUniform3D()
    //             ));

    // static MeshVao geo = readOBJ("ressources/helpers/PointLight.obj", true);

    // helper = newModel(
    //     mat,
    //     geo,
    //     ModelState3D().scaleScalar(1.0)
    // );

    helper = SphereHelperRef(new SphereHelper(vec3(1, 0, 1)));

    add(helper);
}

void PointLightHelper::update(bool forceUpdate)
{
    state.setPosition(light->position());
    state.scaleScalar(light->radius());
    this->ObjectGroup::update(forceUpdate);
}

DirectionalLightHelper::DirectionalLightHelper(SceneDirectionalLight light) : light(light)
{
    GenericSharedBuffer pointsb(new char[sizeof(vec3)*2]);
    vec3* points = (vec3*)pointsb.get();
    points[0] = vec3(0.f);
    points[1] = -light->direction();

    GenericSharedBuffer colorsb(new char[sizeof(vec3)*2]);
    vec3* colors = (vec3*)colorsb.get();
    colors[0] = vec3(0.0f, 0.0f, 1.0f);
    colors[1] = vec3(0.0, 1.0, 0.0);

    MeshVao vao(new 
        VertexAttributeGroup({
            VertexAttribute(pointsb, 0, 2, 3, GL_FLOAT, false),
            VertexAttribute(colorsb, 2, 2, 3, GL_FLOAT, false)
        })
    );

    vao->generate();

    static MeshMaterial material = 
    MeshMaterial(
        new ShaderProgram(
            "shader/foward rendering/basic.frag", 
            "shader/foward rendering/basic.vert", 
            "", 
            globals.standartShaderUniform3D()
            )
        );

    helper = newModel(
        material,
        vao,
        ModelState3D().scaleScalar(1.0)
    );

    helper->defaultMode = GL_LINES;

    add(helper);
}

/*
    Don't work lmao 
    TODO : fix it or remove it from the codebase
*/
void DirectionalLightHelper::update(bool forceUpdate)
{
    // From direction vector to euleur coordinattes
    // No sources, just maths 
    // vec3 dir = -light->direction();
    // float angle1 = atan2(-dir.y, -dir.z); 
    // float angle2 = atan2(-dir.x, dir.y);

    // float angle1 = atan(dir.x/length(vec2(dir.x, dir.z)));
    // float angle2 = atan(-dir.x/dir.z);
    

    // state.setRotation(
    //     vec3(
    //         angle1,
    //         0, // useless
    //         angle2
    //         ));

    vec3 *points = (vec3*)helper->getVao()->attributes[0].getBufferAddr();
    points[0] = vec3(0.f);
    points[1] = -light->direction();

    helper->getVao()->attributes[0].sendAllToGPU();

    this->ObjectGroup::update(forceUpdate);
}

TubeLightHelper::TubeLightHelper(SceneTubeLight light) : light(light)
{
    GenericSharedBuffer pointsb(new char[sizeof(vec3)*2]);
    vec3* points = (vec3*)pointsb.get();
    points[0] = light->position1();
    points[1] = light->position2();

    GenericSharedBuffer colorsb(new char[sizeof(vec3)*2]);
    vec3* colors = (vec3*)colorsb.get();
    colors[0] = light->color();
    colors[1] = light->color();

    MeshVao vao(new 
        VertexAttributeGroup({
            VertexAttribute(pointsb, 0, 2, 3, GL_FLOAT, false),
            VertexAttribute(colorsb, 1, 2, 3, GL_FLOAT, false),
            VertexAttribute(colorsb, 2, 2, 3, GL_FLOAT, false)
        })
    );

    vao->generate();

    static MeshMaterial material = 
    MeshMaterial(
        new ShaderProgram(
            "shader/foward rendering/basic.frag", 
            "shader/foward rendering/basic.vert", 
            "", 
            globals.standartShaderUniform3D()
            )
        );

    helper = newModel(
        material,
        vao,
        ModelState3D().scaleScalar(1.0)
    );

    helper->defaultMode = GL_LINES;
    // helper->depthWrite = false;

    add(helper);
}

/*
    TODO : complete
*/
void TubeLightHelper::update(bool forceUpdate)
{
    MeshVao vao = helper->getVao();

    vec3* points = (vec3*)vao->attributes[0].getBufferAddr();
    points[0] = light->position1();
    points[1] = light->position2();

    vao->attributes[0].sendAllToGPU();

    this->ObjectGroup::update(forceUpdate);
}

CubeHelper::CubeHelper(const vec3 min, const vec3 max, vec3 _color) : MeshModel3D(globals.basicMaterial)
{
    color = _color;
    createUniforms();
    uniforms.add(ShaderUniform(&color, 20));

    // state.frustumCulled = false;
    // depthWrite = false;
    noBackFaceCulling = true;
    defaultMode = GL_LINES;

    int nbOfPoints = 48;
    GenericSharedBuffer buff(new char[sizeof(vec3)*nbOfPoints]);
    GenericSharedBuffer buffNormal(new char[sizeof(vec3)*nbOfPoints]);

    vec3 *pos = (vec3*)buff.get();
    vec3 *nor = (vec3*)buffNormal.get();

    for(int i = 0; i < nbOfPoints; i++)
    {
        nor[i] = vec3(2);
        pos[i] = vec3(0);
    }


    // Face 1
    pos[0] = min*vec3(1, 1, 1) + max*(vec3(0, 0, 0));
    pos[1] = min*vec3(1, 0, 1) + max*(vec3(0, 1, 0));

    pos[2] = min*vec3(1, 1, 1) + max*(vec3(0, 0, 0));
    pos[3] = min*vec3(1, 1, 0) + max*(vec3(0, 0, 1));

    pos[4] = min*vec3(1, 0, 0) + max*(vec3(0, 1, 1));
    pos[5] = min*vec3(1, 1, 0) + max*(vec3(0, 0, 1));

    pos[6] = min*vec3(1, 0, 0) + max*(vec3(0, 1, 1));
    pos[7] = min*vec3(1, 0, 1) + max*(vec3(0, 1, 0));

    // Face 2
    pos[8] = min*vec3(0, 1, 1) + max*(vec3(1, 0, 0));
    pos[9] = min*vec3(0, 0, 1) + max*(vec3(1, 1, 0));

    pos[10] = min*vec3(0, 1, 1) + max*(vec3(1, 0, 0));
    pos[11] = min*vec3(0, 1, 0) + max*(vec3(1, 0, 1));

    pos[12] = min*vec3(0, 0, 0) + max*(vec3(1, 1, 1));
    pos[13] = min*vec3(0, 1, 0) + max*(vec3(1, 0, 1));

    pos[14] = min*vec3(0, 0, 0) + max*(vec3(1, 1, 1));
    pos[15] = min*vec3(0, 0, 1) + max*(vec3(1, 1, 0));

    // Connecting faces
    pos[16] = min*vec3(1, 1, 1) + max*(vec3(0, 0, 0));
    pos[17] = min*vec3(0, 1, 1) + max*(vec3(1, 0, 0));

    pos[18] = min*vec3(1, 0, 1) + max*(vec3(0, 1, 0));
    pos[19] = min*vec3(0, 0, 1) + max*(vec3(1, 1, 0));

    pos[20] = min*vec3(1, 1, 0) + max*(vec3(0, 0, 1));
    pos[21] = min*vec3(0, 1, 0) + max*(vec3(1, 0, 1));

    pos[22] = min*vec3(1, 0, 0) + max*(vec3(0, 1, 1));
    pos[23] = min*vec3(0, 0, 0) + max*(vec3(1, 1, 1));


    // Cross inside faces
    // pos[24] = min*vec3(1, 1, 1) + max*(vec3(0, 0, 0));
    // pos[25] = min*vec3(0, 0, 1) + max*(vec3(1, 1, 0));

    // pos[26] = min*vec3(1, 1, 0) + max*(vec3(0, 0, 1));
    // pos[27] = min*vec3(0, 0, 0) + max*(vec3(1, 1, 1));

    // pos[28] = min*vec3(1, 0, 1) + max*(vec3(0, 1, 0));
    // pos[29] = min*vec3(0, 1, 1) + max*(vec3(1, 0, 0));

    // pos[30] = min*vec3(1, 0, 0) + max*(vec3(0, 1, 1));
    // pos[31] = min*vec3(0, 1, 0) + max*(vec3(1, 0, 1));

    // pos[32] = min*vec3(1, 1, 1) + max*(vec3(0, 0, 0));
    // pos[33] = min*vec3(0, 1, 0) + max*(vec3(1, 0, 1));

    // pos[34] = min*vec3(1, 1, 1) + max*(vec3(0, 0, 0));
    // pos[35] = min*vec3(1, 0, 0) + max*(vec3(0, 1, 1));

    // pos[36] = min*vec3(1, 1, 0) + max*(vec3(0, 0, 1));
    // pos[37] = min*vec3(0, 1, 1) + max*(vec3(1, 0, 0));

    // pos[38] = min*vec3(1, 0, 1) + max*(vec3(0, 1, 0));
    // pos[39] = min*vec3(1, 1, 0) + max*(vec3(0, 0, 1));
    
    // pos[40] = min*vec3(1, 0, 1) + max*(vec3(0, 1, 0));
    // pos[41] = min*vec3(0, 0, 0) + max*(vec3(1, 1, 1));

    // pos[42] = min*vec3(1, 0, 0) + max*(vec3(0, 1, 1));
    // pos[43] = min*vec3(0, 0, 1) + max*(vec3(1, 1, 0));

    // pos[44] = min*vec3(0, 0, 0) + max*(vec3(1, 1, 1));
    // pos[45] = min*vec3(0, 1, 1) + max*(vec3(1, 0, 0));

    // pos[46] = min*vec3(0, 0, 1) + max*(vec3(1, 1, 0));
    // pos[47] = min*vec3(0, 1, 0) + max*(vec3(1, 0, 1));

    MeshVao vao(new 
        VertexAttributeGroup({
            VertexAttribute(buff, 0, nbOfPoints, 3, GL_FLOAT, false),
            VertexAttribute(buffNormal, 1, nbOfPoints, 3, GL_FLOAT, false),
            VertexAttribute(buffNormal, 2, nbOfPoints, 3, GL_FLOAT, false)
        })
    );

    setVao(vao);
}

SphereHelper::SphereHelper(vec3 _color, float radius) : MeshModel3D(globals.basicMaterial)
{
    color = _color;
    createUniforms();
    uniforms.add(ShaderUniform(&color, 20));

    // state.frustumCulled = false;
    // depthWrite = false;
    noBackFaceCulling = true;
    defaultMode = GL_LINES;

    int stepU = 16;
    int stepV = 16;
    int nbOfPoints = stepU*stepV*4;
    GenericSharedBuffer buff(new char[sizeof(vec3)*nbOfPoints]);
    GenericSharedBuffer buffNormal(new char[sizeof(vec3)*nbOfPoints]);

    vec3 *pos = (vec3*)buff.get();
    vec3 *nor = (vec3*)buffNormal.get();


    int id = 0;

    // for(int i = 0; i < nbOfPoints; i++)
    for(int j = 0; j < stepV; j++)
    for(int i = 0; i < stepU; i++)
    {
        vec2 uv = vec2(
            PI2*(float)i/(float)stepU,
            PI2*(float)j/(float)stepV 
        );

        vec2 uv2 = vec2(
            PI2*(float)i/(float)stepU,
            PI2*(float)(j+1)/(float)stepV 
        );

        vec2 uv3 = vec2(
            PI2*(float)(i+1)/(float)stepU,
            PI2*(float)j/(float)stepV 
        );  

        pos[id] = PhiThetaToDir(uv)*radius;
        nor[id] = vec3(2);
        id++;
        pos[id] = PhiThetaToDir(uv2)*radius;
        nor[id] = vec3(2);
        id++;

        pos[id] = PhiThetaToDir(uv)*radius;
        nor[id] = vec3(2);
        id++;
        pos[id] = PhiThetaToDir(uv3)*radius;
        nor[id] = vec3(2);
        id++;
    }

    MeshVao vao(new 
        VertexAttributeGroup({
            VertexAttribute(buff, 0, nbOfPoints, 3, GL_FLOAT, false),
            VertexAttribute(buffNormal, 1, nbOfPoints, 3, GL_FLOAT, false),
            VertexAttribute(buffNormal, 2, nbOfPoints, 3, GL_FLOAT, false)
        })
    );

    setVao(vao);
}

ClusteredFrustumHelper::ClusteredFrustumHelper(Camera cam,  ivec3 dim, vec3 _color) : MeshModel3D(globals.basicMaterial)
{
    color = _color;
    createUniforms();
    uniforms.add(ShaderUniform(&color, 20));

    // state.frustumCulled = false;
    // depthWrite = false;
    noBackFaceCulling = true;
    defaultMode = GL_LINES;

    const int stepD = dim.z; //8
    const int stepX = dim.x;
    const int stepY = dim.y;
    float vFar = 5e3;

    int nbOfPoints = (stepY +1)*(stepX +1)*(stepD+1)*6;
    GenericSharedBuffer buff(new char[sizeof(vec3)*nbOfPoints]);
    GenericSharedBuffer buffNormal(new char[sizeof(vec3)*nbOfPoints]);

    vec3 *pos = (vec3*)buff.get();
    vec3 *nor = (vec3*)buffNormal.get();


    // float k = depth;
    // k = 0.001/k;
    // k -= mod(k, 1.0/nbSlice.z);
    // if(k > 1.0) k = 0.0;

    mat4 m = inverse(cam.updateProjectionViewMatrix());

    int id = 0;

    for(int i = 0; i < nbOfPoints; i++)
    {
        nor[i] = vec3(2);;
        pos[i] = vec3(1e12);
    }

    for(int i = 0; i <= stepD; i++)
    {
        float z = (float)stepD/(max((float)i, 1e-5f)*vFar);
        float z2 = (float)stepD/((float)(i+1)*vFar);

        // z = log(-z/0.1)/log(1 + 2*tan(35)/(float)stepY);
        // z2 = log(-z2/0.1)/log(1 + 2*tan(35)/(float)stepY);

        // z = (float)stepD/max((float)i, 1e-5f);
        // z2 = (float)stepD/((float)(i+1));

        // z = pow(z, 0.5)/vFar;
        // z2 = pow(z2, 0.5)/vFar;


        for(int j = 0; j <= stepX; j++)
        for(int k = 0; k <= stepY; k++)
        {
            float x = 1.0 - 2.0*(float)j/(float)stepX;
            float y = 1.0 - 2.0*(float)k/(float)stepY;

            vec3 p1 = viewToWorld(vec4(-x, -y, z, 1.0), m);
            vec3 p2 = viewToWorld(vec4( x, -y, z, 1.0), m);
            vec3 p3 = viewToWorld(vec4(-x,  y, z, 1.0), m);

            pos[id++] = p1; pos[id++] = p2;
            pos[id++] = p1; pos[id++] = p3;

            if(i != stepD)
            {
                pos[id++] = p1; 
                pos[id++] = viewToWorld(vec4(-x, -y, z2, 1.0), m);
            }
        }
    }

    MeshVao vao(new 
        VertexAttributeGroup({
            VertexAttribute(buff, 0, nbOfPoints, 3, GL_FLOAT, false),
            VertexAttribute(buffNormal, 1, nbOfPoints, 3, GL_FLOAT, false),
            VertexAttribute(buffNormal, 2, nbOfPoints, 3, GL_FLOAT, false)
        })
    );

    setVao(vao);
}

SkeletonHelper::SkeletonHelper(const SkeletonAnimationState &state) : state(state)
{
    int nbOfPoints = 16;
    GenericSharedBuffer buff(new char[sizeof(vec3)*nbOfPoints]);
    GenericSharedBuffer buffNormal(new char[sizeof(vec3)*nbOfPoints]);

    vec3 *pos = (vec3*)buff.get();
    vec3 *nor = (vec3*)buffNormal.get();

    for(int i = 0; i < nbOfPoints; i++)
    {
        nor[i] = vec3(2);
        pos[i] = vec3(0);
    }

    const float b = 0.01;
    const float t = 0.15;

    int id = 0;
    pos[id++] = vec3( b, 0,  b); pos[id++] = vec3(-b, 0,  b);
    pos[id++] = vec3( b, 0,  b); pos[id++] = vec3( b, 0, -b);
    pos[id++] = vec3(-b, 0,  b); pos[id++] = vec3(-b, 0, -b);
    pos[id++] = vec3( b, 0, -b); pos[id++] = vec3(-b, 0, -b);

    pos[id++] = vec3( b, 0,  b); pos[id++] = vec3( 0, t,  0);
    pos[id++] = vec3(-b, 0,  b); pos[id++] = vec3( 0, t,  0);
    pos[id++] = vec3( b, 0, -b); pos[id++] = vec3( 0, t,  0);
    pos[id++] = vec3(-b, 0, -b); pos[id++] = vec3( 0, t,  0);

    // pos[id++] = vec3( b, 0,  0); pos[id++] = vec3( 0, t,  0);
    // pos[id++] = vec3(-0, 0,  b); pos[id++] = vec3( 0, t,  0);
    // pos[id++] = vec3( b, 0, -0); pos[id++] = vec3( 0, t,  0);
    // pos[id++] = vec3(-0, 0, -b); pos[id++] = vec3( 0, t,  0);

    MeshVao vao(new 
        VertexAttributeGroup({
            VertexAttribute(buff, 0, nbOfPoints, 3, GL_FLOAT, false),
            VertexAttribute(buffNormal, 1, nbOfPoints, 3, GL_FLOAT, false),
            VertexAttribute(buffNormal, 2, nbOfPoints, 3, GL_FLOAT, false)
        })
    );

    ModelRef boneHelper = newModel(globals.basicMaterial, vao);
    boneHelper->uniforms.add(ShaderUniform(&color, 20));

    boneHelper->noBackFaceCulling = true;
    boneHelper->defaultMode = GL_LINES;
    boneHelper->depthWrite = false;

    int s = state.size();
    for(int i = 0; i < s; i++)
    {
        bones.push_back(boneHelper->copyWithSharedMesh());
        add(bones.back());
    }
};

void SkeletonHelper::update(bool forceUpdate)
{
    int s = state.size();
    for(int i = 0; i < s; i++)
    {
        bones[i]->state.modelMatrix = state[i];
    }

    ObjectGroup::update();
}