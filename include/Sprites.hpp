
// #include <iostream>
// #include <Shader.hpp>

// #include <VertexBuffer.hpp>

// #include <GL/glew.h>
// #define GLFW_DLL
// #include <GLFW/glfw3.h>

// #include <glm/glm.hpp>

// #include <string>

// /*
//     TODO : create constants.hpp
// */
// #define PI 3.1415926535897932384626433832795
// #define DEGREE_TO_RADIANS PI/180.0

// struct ModelState2D
// {
//     glm::vec2 position = vec2(0.f, 0.f);
//     glm::vec2 scale    = vec2(1.f, 1.f);
//     float dRotation    = 0.f;
//     float depth        = 0.f;
// };

// class ModelMatrix2D : glm::mat4
// {
//     ModelMatrix2D(ModelState2D state)
//     {
//         this->update(state);
//     }

//     ModelMatrix2D* update(ModelState2D state)
//     {
//         float ca = cos(state.dRotation);
//         float sa = cos(state.dRotation);

//         /*
//             Creating a Scaling Rotation Position Matrix
//             assuming the model center is (0, 0)
//         */
//         float mat[16] = 
//         {
//             state.scale.x*ca, -state.scale.y*sa,  0.f        ,  0.f,
//             state.scale.x*sa,  state.scale.y*ca,  0.f        ,  0.f,
//             0.f             ,  0.f             ,  0.f        ,  0.f,
//             state.position.x,  state.position.y,  state.depth,  0.f
//         };

//         memcpy(&this->operator[](0).x, mat, sizeof(mat));

//         return this;
//     }
// };

// using namespace glm;

// struct Quad
// {
//     vec2 points[6] =
//     {
//         vec2(-1.f, -1.f),
//         vec2(-1.f,  1.f),
//         vec2( 1.f, -1.f),
//         vec2( 1.f,  1.f),
//         vec2(-1.f,  1.f),
//         vec2( 1.f, -1.f)
//     };
// };

// struct QuadAttrib
// {
//     vec2 points[6] =
//     {
//         vec2(0.f, 0.f),
//         vec2(0.f, 0.f),
//         vec2(0.f, 0.f),
//         vec2(0.f, 0.f),
//         vec2(0.f, 0.f),
//         vec2(0.f, 0.f)
//     };
// };

// void createQuadAttribute(vec2 attribute, QuadAttrib *destination)
// {
//     *destination = {
//         attribute,
//         attribute,
//         attribute,
//         attribute,
//         attribute,
//         attribute
//     };
// }

// class BatchedQuadBuffer
// {
//     const ShaderProgram *program = nullptr;

//     GLuint vertexVBO;
//     GLuint positionVBO;
//     GLuint scaleVBO;
//     GLuint rotationDepthVBO;

//     GLuint vaoHandle = 0;

//     Quad *vertexBuffer;
//     QuadAttrib *positionBuffer;
//     QuadAttrib *scaleBuffer;
//     QuadAttrib *rotationDepthBuffer;

//     uint64 tail = 0;
//     uint64 size = 0;
//     uint64 chunckSize = 512;
//     uint64 baseSize   = 1024;
    
//     public :

//         BatchedQuadBuffer()
//         {
//             size = baseSize;

//             vertexBuffer        = new Quad[size];
//             positionBuffer      = new QuadAttrib[size];
//             scaleBuffer         = new QuadAttrib[size];
//             rotationDepthBuffer = new QuadAttrib[size];
//         }

//         void setProgram(const ShaderProgram &newProgram)
//             {program = &newProgram;};
        
//         uint64 BatchQuad(ModelState2D const &state)
//         {
//             if(tail >= size) resize();

//             Quad newQuad;
//             float rRotation = state.dRotation*DEGREE_TO_RADIANS;
//             vertexBuffer[tail] = newQuad;
//             createQuadAttribute(state.position,               &positionBuffer[tail]);
//             createQuadAttribute(state.scale,                  &scaleBuffer[tail]);
//             createQuadAttribute(vec2(rRotation, state.depth), &rotationDepthBuffer[tail]);

//             return tail++;
//         };

//         void resize()
//         {
//             uint64 newsize = size + chunckSize;
//             Quad *_vertexBuffer        = new Quad[newsize];
//             QuadAttrib *_positionBuffer      = new QuadAttrib[newsize];
//             QuadAttrib *_scaleBuffer         = new QuadAttrib[newsize];
//             QuadAttrib *_rotationDepthBuffer = new QuadAttrib[newsize];

//             memcpy(_vertexBuffer, vertexBuffer, size*sizeof(Quad));
//             memcpy(_positionBuffer, positionBuffer, size*sizeof(Quad));
//             memcpy(_scaleBuffer, scaleBuffer, size*sizeof(Quad));
//             memcpy(_rotationDepthBuffer, rotationDepthBuffer, size*sizeof(Quad));

//             delete vertexBuffer;
//             delete positionBuffer;
//             delete scaleBuffer;
//             delete rotationDepthBuffer;

//             vertexBuffer = _vertexBuffer;
//             positionBuffer = _positionBuffer;
//             scaleBuffer = _scaleBuffer;
//             rotationDepthBuffer = _rotationDepthBuffer;
//         };

//         void create()
//         {
//             // https://subscription.packtpub.com/book/game-development/9781782167020/1/ch01lvl1sec14/sending-data-to-a-shader-using-vertex-attributes-and-vertex-buffer-objects

//             // GLuint vboHandles[4];
//             // glGenBuffers(2, vboHandles);

//             // vertexVBO        = vboHandles[0];
//             // positionVBO      = vboHandles[1];
//             // scaleVBO         = vboHandles[2];
//             // rotationDepthVBO = vboHandles[3];

//             // //// Passing data to the GPU
//             // glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
//             // glBufferData(GL_ARRAY_BUFFER, sizeof(Quad)*size, vertexBuffer, GL_STATIC_DRAW);

//             // glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
//             // glBufferData(GL_ARRAY_BUFFER, sizeof(Quad)*size, positionBuffer, GL_STATIC_DRAW);

//             // glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
//             // glBufferData(GL_ARRAY_BUFFER, sizeof(Quad)*size, scaleBuffer, GL_STATIC_DRAW);

//             // glBindBuffer(GL_ARRAY_BUFFER, rotationDepthVBO);
//             // glBufferData(GL_ARRAY_BUFFER, sizeof(Quad)*size, rotationDepthBuffer, GL_STATIC_DRAW);


//             // glGenVertexArrays( 1, &vaoHandle );
//             // glBindVertexArray(vaoHandle);

//             // glEnableVertexAttribArray(0); 
//             // glEnableVertexAttribArray(1);
//             // glEnableVertexAttribArray(2);
//             // glEnableVertexAttribArray(3);

//             // glBindVertexBuffer(0, vertexVBO,            0, sizeof(GLfloat)*2);
//             // glBindVertexBuffer(1, positionVBO,          0, sizeof(GLfloat)*2);
//             // glBindVertexBuffer(2, scaleVBO,             0, sizeof(GLfloat)*2);
//             // glBindVertexBuffer(3, rotationDepthVBO,     0, sizeof(GLfloat)*2);
            
//             // glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0);
//             // glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 0);
//             // glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 0);
//             // glVertexAttribFormat(3, 2, GL_FLOAT, GL_FALSE, 0);

//             // glVertexAttribBinding(0, 0);
//             // glVertexAttribBinding(1, 1);
//             // glVertexAttribBinding(2, 2);
//             // glVertexAttribBinding(3, 3);


//             // std::cout << vaoHandle << "\n";
//             // std::cout << vertexVBO << "\n";
//             // std::cout << positionVBO << "\n";
//             // std::cout << scaleVBO << "\n";
//             // std::cout << rotationDepthVBO << "\n";


//             std::shared_ptr<char []> v = std::shared_ptr<char[]>(new char[1024]);
//             std::shared_ptr<char []> p = std::shared_ptr<char[]>(new char[1024]);
//             std::shared_ptr<char []> s = std::shared_ptr<char[]>(new char[1024]);
//             std::shared_ptr<char []> r = std::shared_ptr<char[]>(new char[1024]);

//             memcpy((void*)v.get(), vertexBuffer, 1024);
//             memcpy((void*)p.get(), positionBuffer, 1024);
//             memcpy((void*)s.get(), scaleBuffer, 1024);
//             memcpy((void*)r.get(), rotationDepthBuffer, 1024);

//             // VertexAttribute va(v, 0, 6*tail, 2, GL_FLOAT, false);
//             // VertexAttribute pa(p, 0, 6*tail, 2, GL_FLOAT, false);
//             // VertexAttribute sa(s, 0, 6*tail, 2, GL_FLOAT, false);
//             // VertexAttribute ra(r, 0, 6*tail, 2, GL_FLOAT, false);

//             // va.genBuffer();
//             // va.sendAllToGPU();
//             // pa.genBuffer();
//             // pa.sendAllToGPU();
//             // sa.genBuffer();
//             // sa.sendAllToGPU();
//             // ra.genBuffer();
//             // ra.sendAllToGPU();

//             // glGenVertexArrays(1, &vaoHandle);
//             // glBindVertexArray(vaoHandle);

//             // glEnableVertexAttribArray(0); 
//             // glBindVertexBuffer(0, va.getHandle(), 0, sizeof(GLfloat)*2);
//             // glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0);
//             // glVertexAttribBinding(0, 0);

//             // glEnableVertexAttribArray(1);
//             // glBindVertexBuffer(1, pa.getHandle(), 0, sizeof(GLfloat)*2);
//             // glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 0);
//             // glVertexAttribBinding(1, 1);

//             // glEnableVertexAttribArray(2);
//             // glBindVertexBuffer(2, sa.getHandle(), 0, sizeof(GLfloat)*2);
//             // glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 0);
//             // glVertexAttribBinding(2, 2);

//             // glEnableVertexAttribArray(3);
//             // glBindVertexBuffer(3, ra.getHandle(), 0, sizeof(GLfloat)*2);
//             // glVertexAttribFormat(3, 2, GL_FLOAT, GL_FALSE, 0);
//             // glVertexAttribBinding(3, 3);


//             VertexAttributeGroup attrib(
//                 {
//                     VertexAttribute(v, 0, 6*tail, 2, GL_FLOAT, false),
//                     VertexAttribute(p, 1, 6*tail, 2, GL_FLOAT, false),
//                     VertexAttribute(s, 2, 6*tail, 2, GL_FLOAT, false),
//                     VertexAttribute(r, 3, 6*tail, 2, GL_FLOAT, false)
//                 });
            
//             attrib.generate();

//             vaoHandle = attrib.getHandle();
//         }

//         void draw()
//         {
//             if(tail == 0) return;

//             if(program) program->activate();

//             glBindVertexArray(vaoHandle);
//             glDrawArrays(GL_TRIANGLES, 0, 6*tail);

//             if(program) program->deactivate();
//         }
// };


// class SpriteEngine
// {

// };