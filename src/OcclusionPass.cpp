#include <Graphics/OcclusionPass.hpp>
#include <AssetManager.hpp>
#include <Globals.hpp>

OcclusionPass::OcclusionPass()
{

}

void OcclusionPass::setup()
{
    shader = ShaderProgram(
        Loader<ShaderFragPath>::get("depthOnly").path,
        Loader<ShaderVertPath>::get("basic").path,
        globals.standartShaderUniform3D()
    );

    box.setVao(Loader<MeshVao>::get("QueryBox"));
    box.noBackFaceCulling = true;
}

// void OcclusionPass::add(OcclussionQuery * query)
// {
//     queries.push_back(query);
// }

OcclussionQuery* OcclusionPass::add(vec3 middle, vec3 extents, bool * active)
{
    queries.push_back(OcclussionQuery());
    auto &q = queries.back();
    q.middle = middle;
    q.extents = extents;
    q.active = active;

    return &q;
}

void OcclusionPass::render(Camera &camera)
{
    static BenchTimer timer("Occlusion Pass");

    timer.start();

    int skipped = 0;
    int querried = 0;

    if(isEnable)
    {
        glDisable(GL_CULL_FACE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        // FBO.activate();
        shader.activate();

        // glBindVertexArray(box.getVao()->getHandle());

        int frameLatency = 8;

        int i = globals.appTime.getUpdateCounter()%frameLatency;
        int size = queries.size();

        // for(auto i : queries)
        for(; i < size; i += frameLatency)
        {
            auto &q = queries[i];
            if(q.isQueryActive())
            {
                vec3 camClamp = clamp(camera.getPosition(), q.middle-q.extents, q.middle+q.extents);
                if(camClamp == camera.getPosition())
                {
                    q.containCamera = true;
                    continue;
                }
                else
                {
                    q.containCamera = false;
                }


                // box.state.setPosition(i->middle).setScale(i->extents).update();
                box.state.modelMatrix = mat4(
                    q.extents.x, 0, 0, 0,
                    0, q.extents.y, 0, 0,
                    0, 0, q.extents.z, 0,
                    q.middle.x, q.middle.y, q.middle.z, 1
                );
                box.baseUniforms.update();
                box.uniforms.update();
    
                q.startQuery();
                box.drawVAO(0, true);
                q.stopQuery();
                // i.retreiveQueryResults();


                querried ++;
            }
            else skipped ++;
        }

        shader.deactivate();
        // FBO.deactivate();

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
    }

    timer.stop();

    // std::cout << timer.getLastAvg();
    // std::cout << "\ntotal   : " << queries.size();
    // std::cout << "\nskipped : " << skipped;
    // std::cout << "\nqueried : " << querried ++ << "\n\n";
}