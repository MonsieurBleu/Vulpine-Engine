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

#include <math.h> 
#include <float.h> 

#define PI 3.141592653589793 
/**
 * @param X1x
 * @param X1y
 * @param X1z X1 vector coordinates
 * @param Y1x
 * @param Y1y
 * @param Y1z Y1 vector coordinates
 * @param Z1x
 * @param Z1y
 * @param Z1z Z1 vector coordinates
 * @param pre precession rotation
 * @param nut nutation rotation
 * @param rot intrinsic rotation
 */
void lcs2Euler(
        float X1x, float X1y, float X1z,
        float Y1x, float Y1y, float Y1z,
        float Z1x, float Z1y, float Z1z,
        float *pre, float *nut, float *rot) {
    float Z1xy = sqrt(Z1x * Z1x + Z1y * Z1y);
    if (Z1xy > DBL_EPSILON) {
        *pre = atan2(Y1x * Z1y - Y1y*Z1x, X1x * Z1y - X1y * Z1x);
        *nut = atan2(Z1xy, Z1z);
        *rot = -atan2(-Z1x, Z1y);
    }
    else {
        *pre = 0.;
        *nut = (Z1z > 0.) ? 0. : PI;
        *rot = -atan2(X1y, X1x);
    }
}

void DirectionalLightHelper::update(bool forceUpdate)
{
    // vec3 rot;

    // lcs2Euler(
    //     0, 0, 0,
    //     light->direction().x, light->direction().y, light->direction().z,
    //     0, 1, 0,
    //     &rot.x, &rot.y, &rot.x
    // );

    // state.setRotation(normalize(rot));

    // state.setRotation(
    //     -acos(light->direction())
    //     // acos(light->direction()) * sign(asin(light->direction()))
    //     );

    vec3 dir = light->direction();
    float angle1 = acos(dir.y)*sign(asin(dir.z));
    float angle2 = -acos(dir.y)*sign(asin(dir.x));

    state.setRotation(
        vec3(
            angle1,
            0, // useless
            angle2
            ));

    //meshes[0]->state.lookAt(light->direction()*50.0);

    // meshes.front()->state.lookAt(light->direction());

    this->ObjectGroup::update(forceUpdate);
}