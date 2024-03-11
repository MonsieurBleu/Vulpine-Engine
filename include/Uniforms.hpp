#ifndef UNIFORMS_HPP
#define UNIFORMS_HPP

// #include <GLutils.hpp>
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include <Matrix.hpp>

#define STATIC_CAST_VOIDPTR *(void**)(int *)&
#define STATIC_CAST_FLOAT   *(float *)&

using namespace glm;

typedef glm::vec<2, uint32, glm::packed_highp> uivec2;
typedef glm::vec<3, uint32, glm::packed_highp> uivec3;
typedef glm::vec<4, uint32, glm::packed_highp> uivec4;

enum UniformTypes
{
    unknownUniformType,

    _1f,
    _2f,
    _3f,
    _4f,

    _1i,
    _2i,
    _3i,
    _4i,

    _1ui,
    _2ui,
    _3ui,
    _4ui,

    _1fv,
    _2fv,
    _3fv,
    _4fv,

    _1iv,
    _2iv,
    _3iv,
    _4iv,

    _1uiv,
    _2uiv,
    _3uiv,
    _4uiv,

    Matrix2fv,
    Matrix3fv,
    Matrix4fv,

    Matrix2x3fv,
    Matrix3x2fv,

    Matrix2x4fv,
    Matrix4x2fv,

    Matrix3x4fv,
    Matrix4x3fv
};

const std::string UniformTypesNames[] = 
{
    "unknownUniformType",

    "_1f",
    "_2f",
    "_3f",
    "_4f",

    "_1i",
    "_2i",
    "_3i",
    "_4i",

    "_1ui",
    "_2ui",
    "_3ui",
    "_4ui",

    "_1fv",
    "_2fv",
    "_3fv",
    "_4fv",

    "_1iv",
    "_2iv",
    "_3iv",
    "_4iv",

    "_1uiv",
    "_2uiv",
    "_3uiv",
    "_4uiv",

    "Matrix2fv",
    "Matrix3fv",
    "Matrix4fv",

    "Matrix2x3fv",
    "Matrix3x2fv",

    "Matrix2x4fv",
    "Matrix4x2fv",

    "Matrix3x4fv",
    "Matrix4x3fv"
};

#define UNIFORM_TYPES_PTR_DATA_THRESHOLD _1fv // unused
#define UNIFORM_NO_LOCATION 0xffff

enum UniformDataState
{
    unknownUniformDataState,
    rawCopy,
    copiedInAdditionalData,
    reference
};

const std::string UniformDataStateNames[] =
{
    "unknownUniformDataState",
    "rawCopy",
    "copiedInAdditionalData",
    "reference"
};

int getUniformTypeSize( UniformTypes type);

class ShaderUniform
{
    friend std::ostream& operator<<(std::ostream&, ShaderUniform);

    const void *data;
    uint64 additionalData[8];
    UniformDataState dataState = unknownUniformDataState;

    int  location;
    UniformTypes type;

    int _count = 1;

    public :

        int getLocation() const {return location;};

        void activate() const;

        void setData(const void *newData){data = newData;};

        ShaderUniform& setCount(int count);

        // ============| DEFAULT NO DATA

        ShaderUniform()
            : data(nullptr), 
              dataState(unknownUniformDataState),
              location(UNIFORM_NO_LOCATION),
              type(unknownUniformType){};


        // ============| FLOATS 1D

        ShaderUniform(const float data, int location = UNIFORM_NO_LOCATION) 
            : data(STATIC_CAST_VOIDPTR data), 
              dataState(rawCopy),
              location(location),
              type(_1f) {};

        ShaderUniform(const float *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(_1f) {};        


        // ============| FLOATS 2D

        ShaderUniform(const vec2 data, int location = UNIFORM_NO_LOCATION) 
            : dataState(copiedInAdditionalData),
              location(location),
              type(_2fv)
            {
                memcpy(additionalData, (void*)&data, sizeof(vec2));
            };

        ShaderUniform(const vec2 *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(_2fv) {};


        // ============| FLOATS 3D

        ShaderUniform(const vec3 data, int location = UNIFORM_NO_LOCATION) 
            : dataState(copiedInAdditionalData),
              location(location),
              type(_3fv)
            {
                memcpy(additionalData, (void*)&data, sizeof(vec3));
            };

        ShaderUniform(const vec3 *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(_3fv) {};

        // ============| FLOATS 4D

        ShaderUniform(const vec4 cdata, int location = UNIFORM_NO_LOCATION) 
            : dataState(copiedInAdditionalData),
              location(location),
              type(_4fv)
            {
                memcpy(additionalData, (void*)&cdata, sizeof(vec4));
            };

        ShaderUniform(const vec4 *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(_4fv) {};


        // ============| INTS 1D

        ShaderUniform(const int data, int location = UNIFORM_NO_LOCATION) 
            : data(STATIC_CAST_VOIDPTR data), 
              dataState(rawCopy),
              location(location),
              type(_1i) {};

        ShaderUniform(const int *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(_1i) {};        


        // ============| INTS 2D

        ShaderUniform(const ivec2 data, int location = UNIFORM_NO_LOCATION) 
            : dataState(copiedInAdditionalData),
              location(location),
              type(_2iv)
            {
                memcpy(additionalData, (void*)&data, sizeof(ivec2));
            };

        ShaderUniform(const ivec2 *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(_2iv) {};


        // ============| INTS 3D

        ShaderUniform(const ivec3 data, int location = UNIFORM_NO_LOCATION) 
            : dataState(copiedInAdditionalData),
              location(location),
              type(_3iv)
            {
                memcpy(additionalData, (void*)&data, sizeof(ivec3));
            };

        ShaderUniform(const ivec3 *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(_3iv) {};

        // ============| INTS 4D

        ShaderUniform(const ivec4 data, int location = UNIFORM_NO_LOCATION) 
            : dataState(copiedInAdditionalData),
              location(location),
              type(_4iv)
            {
                memcpy(additionalData, (void*)&data, sizeof(ivec4));
            };

        ShaderUniform(const ivec4 *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(_4iv) {};

        // ============| UNSIGNED INTS 1D

        ShaderUniform(const uint32 data, int location = UNIFORM_NO_LOCATION) 
            : data(STATIC_CAST_VOIDPTR data), 
              dataState(rawCopy),
              location(location),
              type(_1ui) {};

        ShaderUniform(const uint32 *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(_1ui) {};        


        // ============| UNSIGNED INTS 2D

        ShaderUniform(const uivec2 data, int location = UNIFORM_NO_LOCATION) 
            : dataState(copiedInAdditionalData),
              location(location),
              type(_2uiv)
            {
                memcpy(additionalData, (void*)&data, sizeof(uivec2));
            };

        ShaderUniform(const uivec2 *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(_2uiv) {};


        // ============| UNSIGNED INTS 3D

        ShaderUniform(const uivec3 data, int location = UNIFORM_NO_LOCATION) 
            : dataState(copiedInAdditionalData),
              location(location),
              type(_3uiv)
            {
                memcpy(additionalData, (void*)&data, sizeof(uivec3));
            };

        ShaderUniform(const uivec3 *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(_3uiv) {};

        // ============| UNSIGNED INTS 4D

        ShaderUniform(const uivec4 data, int location = UNIFORM_NO_LOCATION) 
            : dataState(copiedInAdditionalData),
              location(location),
              type(_4uiv)
            {
                memcpy(additionalData, (void*)&data, sizeof(uivec4));
            };

        ShaderUniform(const uivec4 *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(_4uiv) {};
        
        // ============| MATRIX 2
        ShaderUniform(const mat2 data, int location = UNIFORM_NO_LOCATION) 
            : dataState(copiedInAdditionalData),
              location(location),
              type(Matrix2fv)
            {
                memcpy(additionalData, (void*)&data, sizeof(mat2));
            };

        ShaderUniform(const mat2 *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(Matrix2fv) {};

        // ============| MATRIX 3
        ShaderUniform(const mat3 data, int location = UNIFORM_NO_LOCATION) 
            : dataState(copiedInAdditionalData),
              location(location),
              type(Matrix3fv)
            {
                memcpy(additionalData, (void*)&data, sizeof(mat3));
            };

        ShaderUniform(const mat3 *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(Matrix3fv) {};

        // ============| MATRIX 4
        ShaderUniform(const mat4 data, int location = UNIFORM_NO_LOCATION) 
            : dataState(copiedInAdditionalData),
              location(location),
              type(Matrix4fv)
            {
                memcpy(additionalData, (void*)&data, sizeof(mat4));
            };

        ShaderUniform(const mat4 *data, int location = UNIFORM_NO_LOCATION) 
            : data(data), 
              dataState(reference),
              location(location),
              type(Matrix4fv) {};

        // ShaderUniform(const ModelMatrix3D data, int location = UNIFORM_NO_LOCATION) 
        //     :   
        //       dataState(copiedInAdditionalData),
        //       location(location),
        //       type(Matrix4fv)
        //     {
        //         memcpy(additionalData, (void*)&data, sizeof(mat4));
        //     };

        // ShaderUniform(const ModelMatrix3D *data, int location = UNIFORM_NO_LOCATION) 
        //     : data(data), 
        //       dataState(reference),
        //       location(location),
        //       type(Matrix4fv) {};



};


/*
    TODO : 
        Add the possibility to add, change or remove things from the group
        Add a copy constructor
*/
class ShaderUniformGroup
{
    friend std::ostream& operator<<(std::ostream& os, ShaderUniformGroup g);

    private : 
        std::vector<ShaderUniform> uniforms;

        bool safe = false;
        bool autoCheckLocations = true;

        void sort();

    public : 

        ShaderUniformGroup(bool autoCheckLocations = true) : autoCheckLocations(autoCheckLocations) {};

        ShaderUniformGroup(
            std::vector<ShaderUniform> uniforms,
            bool autoCheckLocations = true
        );

        /*
            TODO : 
                Add enum to check if the uniform group is : 
                    safe : no bad or duplicate locations
                    ambigious : locations doesn't correspond to their position in the buffer
                    uncomplete : there is a gap in the uniforms 
        */
        bool checkLocations();

        void update() const;

        void forEach(void (*func)(int, ShaderUniform&));
        void forEach(void (*func)(int, const ShaderUniform&)) const;

        void add(ShaderUniform newUniform);
};

std::ostream& operator<<(std::ostream& os, ShaderUniform u);

std::ostream& operator<<(std::ostream& os, ShaderUniformGroup g);

std::vector<ShaderUniform>& operator+(std::vector<ShaderUniform>& a, ShaderUniform &b);

std::vector<ShaderUniform>& operator+(std::vector<ShaderUniform>& a, std::vector<ShaderUniform>& b);

#endif