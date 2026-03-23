#pragma once

#include <Graphics/Shader.hpp>
#include <VertexBuffer.hpp>
#include <Graphics/Textures.hpp>

#include <Graphics/Mesh.hpp>

class OcclusionPass;

class OcclussionQuery
{
    private : 
        GLint id = -1;
        GLint occluded = 0;

        // static MeshVao cubeVao;

        friend OcclusionPass;
        
    public : 
        // OcclussionQuery(vec3 middle, vec3 extents, bool * active);
        OcclussionQuery();
        ~OcclussionQuery();

        vec3 middle = vec3(0);
        vec3 extents = vec3(0);

    private :  
        bool *active = nullptr;
        bool containCamera = false;

    public:
        void startQuery();
        void stopQuery();
        void retreiveQueryResults();

        GLuint getQueryId(){return id;};
        GLint getQueryResult();
        bool isQueryActive(int layer = 0);
};