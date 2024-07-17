#pragma once

#include <ObjectGroup.hpp>
#include <Skeleton.hpp>
#include <NavGraph.hpp>
#include <Fonts.hpp>
#include <AssetManager.hpp>
#include <Globals.hpp>

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

class LineHelper : public MeshModel3D
{
    private :

    public : 
        vec3 color;
        LineHelper(const vec3 min, const vec3 max, vec3 _color = vec3(0, 1, 0));
};

typedef std::shared_ptr<LineHelper> LineHelperRef;

class CubeHelper : public MeshModel3D
{
    private :

    public : 
        vec3 color;
        
        CubeHelper(const vec3 min, const vec3 max, vec3 _color = vec3(0, 1, 0));

        void updateData(const vec3 min, const vec3 max);
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

class CapsuleHelper : public MeshModel3D
{
    private : 

        vec3  *pos1;
        vec3  *pos2;
        float *radius;

    public : 
        vec3 color;
        CapsuleHelper(
            vec3  *pos1, 
            vec3  *pos2, 
            float *radius, 
            vec3  color = vec3(0, 1, 0));

        void update();

        void updateData(const vec3 pos1, const vec3 pos2, const float radius);
};

typedef std::shared_ptr<CapsuleHelper> CapsuleHelperRef;

class ClusteredFrustumHelper : public MeshModel3D
{
    private :

    public : 
        vec3 color;
        ClusteredFrustumHelper(Camera cam, ivec3 dim = ivec3(16, 9, 24), vec3 _color = vec3(0, 1, 0));
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

class SkeletonHelper : public ObjectGroup
{
    private : 
        std::vector<ModelRef> bones;
        const SkeletonAnimationState &state;

    public :
        vec3 color = vec3(1, 0, 0);
        SkeletonHelper(const SkeletonAnimationState &state);

        void update(bool forceUpdate = true) override;
};

typedef std::shared_ptr<SkeletonHelper> SkeletonHelperRef;

class PolyhedronHelper : public MeshModel3D
{
    private :

    public : 
        vec3 color;
        PolyhedronHelper(const std::vector<vec3>& points, vec3 color = vec3(0, 1, 0));
};

typedef std::shared_ptr<PolyhedronHelper> PolyhedronHelperRef;

class PointsHelper : public MeshModel3D
{
    private :

    public : 
        vec3 color;
        PointsHelper(const std::vector<vec3>& points, vec3 _color);
};

typedef std::shared_ptr<PointsHelper> PointsHelperRef;

class NavGraphHelper : public ObjectGroup
{
    private : 

        NavGraphRef graph;

    public : 

        NavGraphHelper(NavGraphRef graph);
        
        // void update(bool forceUpdate = true) override;
};  


typedef std::shared_ptr<NavGraphHelper> NavGraphHelperRef;

class PathHelper : public ObjectGroup
{
    private : 
        Path path; 
        NavGraphRef graph;

        static inline int maxPath = 32;

    public : 
        PathHelper(Path path, NavGraphRef graph);
        void update(bool forceUpdate = true) override;
};

typedef std::shared_ptr<PathHelper> PathHelperRef;

template<typename T>
class ValueHelper : public SingleStringBatch
{
    private : 

    public : 
        T &val;
        std::u32string baseText;
        vec3 color;
        ValueHelper(T &val, const std::u32string &btext, vec3 color) : val(val), baseText(btext), color(color)
        {
            setMaterial(Loader<MeshMaterial>::get("BasicFont3D"));
            text = btext;
            setFont(globals.baseFont);
            uniforms.add(ShaderUniform(&this->color, 20));
            align = CENTERED;
            batchText();
            state.frustumCulled = false;
            noBackFaceCulling = true;
        };
        
        void update()
        {
            UFT32Stream s;
            s << baseText; s << val;
            text = s.str();
            batchText();

            SingleStringBatch::update();
        }
};

template<typename T>
class ValueHelperRef : public std::shared_ptr<ValueHelper<T>>
{
    public :
        ValueHelperRef(){};
        ValueHelperRef(ValueHelper<T> *ptr) : std::shared_ptr<ValueHelper<T>>(ptr){};
};

