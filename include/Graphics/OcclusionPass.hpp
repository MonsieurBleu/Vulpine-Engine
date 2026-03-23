#pragma once
#include <Graphics/Query.hpp>
#include <Graphics/RenderPass.hpp>

#include <deque>

class OcclusionPass : public RenderPass
{
    private :
        std::deque<OcclussionQuery> queries;

        MeshModel3D box;

    public :
        OcclusionPass();
        void setup();
        void render(Camera &camera);

        OcclussionQuery* add(vec3 middle, vec3 extents, bool * active);
};