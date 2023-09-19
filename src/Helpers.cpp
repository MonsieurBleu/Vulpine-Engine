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

PointLightHelper::PointLightHelper(ScenePointLight light) : light(light)
{
    setVao(readOBJ("ressources/helpers/PointLight.obj"));
    setMaterial(MeshMaterial(
        new ShaderProgram(
            "shader/foward rendering/phong.frag", 
            "shader/foward rendering/phong.vert", 
            "", 
            globals.standartShaderUniform3D())));
}

void PointLightHelper::drawVAO(GLenum mode)
{
    state.setPosition(light->position());
    state.scaleScalar(light->radius());
    // state.forceUpdate();
    // material->uniforms.update();
    std::cout << state.scale.x << "\n";
    std::cout << uniforms;
    this->MeshModel3D::drawVAO(mode);
}

// void PointLightHelper::draw(GLenum mode)
// {
//     material->activate();
//     PointLightHelper::drawVAO();
//     material->deactivate();
// }