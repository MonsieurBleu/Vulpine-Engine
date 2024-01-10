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

class PointLightHelper : public ObjectGroup
{
    private : 

        const ScenePointLight light;

    public :
        ModelRef helper;

        PointLightHelper(ScenePointLight light);
        void update(bool forceUpdate = false) override;
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



#endif