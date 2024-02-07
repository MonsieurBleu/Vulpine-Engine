#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <ObjectGroup.hpp>

class PointLightHelperMODEL : public MeshModel3D
{
    private : 

        const ScenePointLight light;

    public :

        PointLightHelperMODEL(ScenePointLight light);
        // void update(bool forceUpdate = false) override;
        // void drawVAO(GLenum mode = GL_TRIANGLES);
        // void draw(GLenum mode = GL_TRIANGLES);

        // void preDrawRoutine() override;
};

class DirectionalLightHelper : public ObjectGroup
{
    private : 

        const SceneDirectionalLight light;
        ModelRef helper;

    public :

        DirectionalLightHelper(SceneDirectionalLight light);
        void update(bool forceUpdate = false) override;
};

class TubeLightHelper : public ObjectGroup
{
    private : 

        const SceneTubeLight light;
        ModelRef helper;

    public :

        TubeLightHelper(SceneTubeLight light);
        void update(bool forceUpdate = false) override;
};

class CubeHelper : public MeshModel3D
{
    private :

    public : 
        vec3 color;
        CubeHelper(const vec3 min, const vec3 max, vec3 _color = vec3(0, 1, 0));
};

typedef std::shared_ptr<CubeHelper> CubeHelperRef;

class SphereHelper : public MeshModel3D
{
    private :

    public : 
        vec3 color;
        SphereHelper(vec3 _color = vec3(0, 1, 0), float radius = 1.f);
};

typedef std::shared_ptr<SphereHelper> SphereHelperRef;


class ClusteredFrustumHelper : public MeshModel3D
{
    private :

    public : 
        vec3 color;
        ClusteredFrustumHelper(Camera cam, vec3 _color = vec3(0, 1, 0));
};

typedef std::shared_ptr<ClusteredFrustumHelper> ClusteredFrustumHelperRef;

class PointLightHelper : public ObjectGroup
{
    private : 

        const ScenePointLight light;

    public :
        SphereHelperRef helper;

        PointLightHelper(ScenePointLight light);
        void update(bool forceUpdate = false) override;
};

typedef std::shared_ptr<PointLightHelper> PointLightHelperRef;

#endif