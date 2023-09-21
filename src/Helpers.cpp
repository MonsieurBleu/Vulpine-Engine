#include <Helpers.hpp>
#include <Globals.hpp>
#include <iostream>

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
        readOBJ("ressources/helpers/PointLight.obj"),
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