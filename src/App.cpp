#include <Globals.hpp>

#include <Utils.hpp>
#include <string.h> // for memset

#include <fstream>

#include <Timer.hpp>
#include <Sprites.hpp>

#include <Uniforms.hpp>

#include <Mesh.hpp>

//https://antongerdelan.net/opengl/hellotriangle.html

App::App(GLFWwindow* window) : window(window)
{
    timestart = Get_time_ms();

    /*
        TODO : 
            Test if the videoMode automaticlly update
    */
    globals._videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    globals._standartShaderUniform2D =
    {
        ShaderUniform((vec2 *)globals.screenResolutionAddr(), 0),
        ShaderUniform(globals.appTime.getElapsedTimeAddr(),   1),
    };

    globals._standartShaderUniform3D =
    {
        ShaderUniform((vec2 *)globals.screenResolutionAddr(), 0),
        ShaderUniform(globals.appTime.getElapsedTimeAddr(),   1),
        ShaderUniform(camera.getProjectionViewMatrixAddr(),   2)
    };
}

void App::mainInput(double deltatime)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        state = quit;
    
    if(glfwGetKey(window, GLFW_KEY_F8) == GLFW_PRESS)
    {
        auto myfile = std::fstream("saves/cameraState.bin", std::ios::out | std::ios::binary);
        myfile.write((char*)&camera.getState(), sizeof(CameraState));
        myfile.close();
    }

    float camspeed = 15.0;
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camspeed *= 10.0;

    vec3 velocity(0.0);

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        velocity.x += camspeed;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        velocity.x -= camspeed;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        velocity.z -= camspeed;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        velocity.z += camspeed;
    }
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        velocity.y += camspeed;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        velocity.y -= camspeed;
    }

    camera.move(velocity, deltatime);

    // if(glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
    // {
    //     camera.add_FOV(-0.1);
    // }

    // if(glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
    // {
    //     camera.add_FOV(0.1);
    // }

    // if(glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
    // {
    //     // glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data); //to update partially
    // }

    // if(glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
    // {
    //     camera.toggle_mouse_follow();
    // }   
}

void App::mainloopStartRoutine()
{
    globals.appTime.start();
}

void App::mainloopPreRenderRoutine()
{
    camera.updateProjectionViewMatrix();
}

void App::mainloopEndRoutine()
{
    glfwSwapBuffers(window);
    globals.appTime.end();
}

void App::mainloop()
{   
    /// CENTER WINDOW
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos(window, (mode->width - 1920) / 2, (mode->height - 1080) / 2);

    /// SETTING UP THE CAMERA 
    camera.init(radians(50.0f), 1920.f, 1080.f, 0.1f, 10000.0f);

    camera.setCameraPosition(vec3(4.f, 3.f, 3.f));
    // camera.setForceLookAtPoint(true);
    // camera.lookAt(vec3(0.0));

    auto myfile = std::fstream("saves/cameraState.bin", std::ios::in | std::ios::binary);
    if(myfile)
    {
        CameraState buff;
        myfile.read((char*)&buff, sizeof(CameraState));
        myfile.close();
        camera.setState(buff);
    }


    /// CREATING A SHADER PROGRAM
    // ShaderProgram test("shader/test.frag", "shader/test.vert", "");
    // ShaderProgram test("shader/Voxel.frag", "shader/Voxel.vert", "shader/Voxel.geom");
    ShaderProgram shader("shader/test.frag", "shader/test.vert", "", 
    globals.standartShaderUniform3D());

    shader.activate();
    int winsizeh[2] = {1920/2, 1080/2};
    glUniform2iv(0, 1, winsizeh);

    glEnable(GL_DEPTH_TEST);


    /// CREATINHG VBO
    GLuint vbo;
    glGenBuffers(1, &vbo);

    float voxel_size = 5.0;
    float points[] = 
    {
        // // X FACES
        // -voxel_size, voxel_size, voxel_size,
        // -voxel_size, voxel_size, -voxel_size,
        // -voxel_size, -voxel_size, voxel_size,
        // -voxel_size, -voxel_size, -voxel_size,
        // -voxel_size, voxel_size, -voxel_size,
        // -voxel_size, -voxel_size, voxel_size,

        // voxel_size, voxel_size, voxel_size,
        // voxel_size, voxel_size, -voxel_size,
        // voxel_size, -voxel_size, voxel_size,
        // voxel_size, -voxel_size, -voxel_size,
        // voxel_size, voxel_size, -voxel_size,
        // voxel_size, -voxel_size, voxel_size,

        // Y FACES
        voxel_size, voxel_size, voxel_size,
        voxel_size, voxel_size, -voxel_size,
        -voxel_size, voxel_size, voxel_size,
        -voxel_size, voxel_size, -voxel_size,
        voxel_size, voxel_size, -voxel_size,
        -voxel_size, voxel_size, voxel_size,

        // voxel_size, -voxel_size, voxel_size,
        // voxel_size, -voxel_size, -voxel_size,
        // -voxel_size, -voxel_size, voxel_size,
        // -voxel_size, -voxel_size, -voxel_size,
        // voxel_size, -voxel_size, -voxel_size,
        // -voxel_size, -voxel_size, voxel_size,
    
        // // Z FACES
        // voxel_size, voxel_size, voxel_size,
        // voxel_size, -voxel_size, voxel_size,
        // -voxel_size, voxel_size, voxel_size,
        // -voxel_size, -voxel_size, voxel_size,
        // voxel_size, -voxel_size, voxel_size,
        // -voxel_size, voxel_size, voxel_size,

        // voxel_size, voxel_size, -voxel_size,
        // voxel_size, -voxel_size, -voxel_size,
        // -voxel_size, voxel_size, -voxel_size,
        // -voxel_size, -voxel_size, -voxel_size,
        // voxel_size, -voxel_size, -voxel_size,
        // -voxel_size, voxel_size, -voxel_size,
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,  sizeof(points), points, GL_STATIC_DRAW);

    // Création du VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Spécification de l'agencement des données
    GLint posAttrib = glGetAttribLocation(shader.get_program(), "Vpos");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);



    /// CREATING INSTANCED OFFSETS
    // GLuint pbo;
    // glGenBuffers(1, &pbo);
    // glBindBuffer(GL_ARRAY_BUFFER, pbo);

    // int voxel_elem_nb = 3;
    uint32 grid_size = 1000;
    uint32 instanced_cnt = grid_size*grid_size;
    // float offsets[instanced_cnt*voxel_elem_nb];
    // uint32 index = 0;

    // float voxel_spacement = 3.0;

    // for(index = 0; index < instanced_cnt; index+= voxel_elem_nb)
    // {
    //     offsets[index] = (index/3)*voxel_spacement;
    //     offsets[index+1] = 0.0;
    //     offsets[index+2] = 0.0;
    // }

    // glBufferData(GL_ARRAY_BUFFER, sizeof(offsets), offsets, GL_STATIC_DRAW);
    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // glVertexAttribDivisor(1, 1); // This sets the vertex attribute to instanced attribute.


    ShaderProgram BQBtestShader("shader/test2d.frag", "shader/test2d.vert", "", globals.standartShaderUniform2D());
    BatchedQuadBuffer BQBtest;
    BQBtest.setProgram(BQBtestShader);

    // ModelState2D newQuad;
    // newQuad.position.x = 1.0;
    // newQuad.scale = vec2(0.5, 0.5);
    // newQuad.depth = 0.5;
    // BQBtest.BatchQuad(newQuad);
    

    // BQBtest.BatchQuad({
    //         {0.f, 0.f},
    //         {0.75, 0.75},
    //         3.1415, 
    //         0.f
    //     });

    // BQBtest.BatchQuad({
    //         {0.f, 0.f},
    //         {1.f, 1.f},
    //         3.1415*0.5, 
    //         0.25
    //     });

    // BQBtest.create();
    

    
    MeshMaterial meshShader(
        new ShaderProgram(
            "shader/meshTest.frag", 
            "shader/meshTest.vert", 
            "", 
            globals.standartShaderUniform3D()));

    uint64 vertexCount = 3;

    MeshVao meshVao(
        new VertexAttributeGroup(
            {
                VertexAttribute
                (
                    GenericSharedBuffer((char *)new float[3*3]
                    {
                        0.f, 0.f, 0.f,
                        10.f, 0.f, 0.f,
                        0.f, 10.f, 0.f,
                    }),
                    0,
                    vertexCount,
                    3,
                    GL_FLOAT,
                    false
                ),
                VertexAttribute
                (
                    GenericSharedBuffer((char *)new float[3*3]
                    {
                        1.f, 0.f, 0.f,
                        0.f, 1.f, 0.f,
                        0.f, 0.f, 1.f,
                    }),
                    1,
                    vertexCount,
                    3,
                    GL_FLOAT,
                    false
                )
            }
        ));

    Mesh mesh(meshShader, meshVao);


    while(state != quit)
    {
        mainloopStartRoutine();
        
        glfwPollEvents();

        camera.updateMouseFollow(window);

        if(glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS)
        {
            BQBtestShader.CompileAndLink();

            system("cls");
            glDeleteProgram(shader.get_program());
            shader.CompileAndLink();
            shader.activate();
        }
        if(glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
            std::cout << globals.appTime << "\n";

        mainInput(globals.appTime.getDelta());

        mainloopPreRenderRoutine();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

        shader.activate();

        glBindVertexArray(vao);

        glDrawArraysInstanced(GL_TRIANGLES, 0, sizeof(points)/3, instanced_cnt);

        // BQBtest.draw();

        mesh.draw();

        mainloopEndRoutine();
    }
}