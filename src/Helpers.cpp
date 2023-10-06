#include <Helpers.hpp>
#include <Globals.hpp>
#include <iostream>
#include <Globals.hpp>

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

void PointLightHelperMODEL::preDrawRoutine()
{
    // state.setPosition(vec3(10, 1, 10));
    // state.scaleScalar(5.0);
}

PointLightHelper::PointLightHelper(ScenePointLight light) : light(light)
{
    ModelRef helper = newModel(
        MeshMaterial(
            new ShaderProgram(
                "shader/foward rendering/basic.frag", 
                "shader/foward rendering/basic.vert", 
                "", 
                globals.standartShaderUniform3D()
                )
            ),
        readOBJ("ressources/helpers/PointLight.obj", true),
        ModelState3D().scaleScalar(1.0)
    );

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
    ModelRef helper = newModel(
        MeshMaterial(
            new ShaderProgram(
                "shader/foward rendering/basic.frag", 
                "shader/foward rendering/basic.vert", 
                "", 
                globals.standartShaderUniform3D()
                )
            ),
        readOBJ("ressources/helpers/DirectionnalLight.obj"),
        ModelState3D().scaleScalar(1.0)
    );

    add(helper);

    state.setPosition(vec3(0, 1, 0));
}

/*
    Don't work lmao 
    TODO : fix it or remove it from the codebase
*/
void DirectionalLightHelper::update(bool forceUpdate)
{
    // From direction vector to euleur coordinattes
    // No sources, just maths 
    vec3 dir = -light->direction();
    // float angle1 = atan2(-dir.y, -dir.z); 
    // float angle2 = atan2(-dir.x, dir.y);

    float angle1 = atan(dir.x/length(vec2(dir.x, dir.z)));
    float angle2 = atan(-dir.x/dir.z);
    

    state.setRotation(
        vec3(
            angle1,
            0, // useless
            angle2
            ));

    this->ObjectGroup::update(forceUpdate);
}