#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <Matrix.hpp>
#include <FrameBuffer.hpp>
#include <Camera.hpp>

using namespace glm;

#define MAX_LIGHT_COUNTER 0b100000000000000
#define MAX_LIGHT_PER_CLUSTER 128
#define LIGHT_SHADOW_ACTIVATED 1

class ObjectGroup;
class FastUI_valueMenu;

enum lightTypes
{
    NO_LIGHT,
    DIRECTIONAL_LIGHT,
    POINT_LIGHT,
    TUBE_LIGHT
};

struct LightInfos
{
    vec4 _position = vec4(0);
    vec4 _color = vec4(1);
    vec4 _direction = vec4(0);
    ivec4 _infos = vec4(0); // a : type, b : shadows
    mat4 _rShadowMatrix = mat4(0); 
};

class Light
{
    friend std::ostream& operator<<(std::ostream&, const Light &);
    friend std::basic_ostringstream<char32_t>& operator<<(std::basic_ostringstream<char32_t>& os, const Light &l);
    friend ObjectGroup;

    protected :

        LightInfos infos;

        int shadowMapBindId = -1;
        static const int maxShadowMap = 16;
        static const int shadowMapBaseBind = 16;
        static bool freeShadowMapBinds[maxShadowMap];
    
    public :
        const void* getAttribAddr() const;
        LightInfos getInfos() const;
        virtual void applyModifier(const ModelState3D& state);
        
        Camera shadowCamera;
        FrameBuffer shadowMap; 
        ivec2 cameraResolution = vec2(1024);
        void activateShadows();
        void bindShadowMap();
        virtual void updateShadowCamera();
};  

class DirectionLight : public Light
{
    private : 

        vec3 tmpDirection = vec3(0);

    public :

        DirectionLight();
        DirectionLight& setDirection(vec3 direction);
        DirectionLight& setColor(vec3 color);
        DirectionLight& setIntensity(float intensity);

        vec3 direction();

        void applyModifier(const ModelState3D& state) override;

        void updateShadowCamera();
        vec2 shadowCameraSize;

        void setMenu(FastUI_valueMenu &menu, std::u32string name);
};

class PointLight : public Light
{
    private :

        vec3 tmpPosition = vec3(0);
        float tmpRadius = 1.0;
    
    public :
        PointLight();
        PointLight& setPosition(vec3 position);
        PointLight& setColor(vec3 color);
        PointLight& setRadius(float radius);
        PointLight& setIntensity(float intensity);

        vec3 position() const;
        vec3 color() const;
        float radius() const;
        float intensity() const;

        void applyModifier(const ModelState3D& state) override;
};

class TubeLight : public Light
{
    private : 
        vec3 tmpPos1 = vec3(0);
        vec3 tmpPos2 = vec3(0);
        float tmpRadius = 1.0;
    
    public : 
        TubeLight();
        TubeLight& setPos(vec3 p1, vec3 p2);
        TubeLight& setColor(vec3 color);
        TubeLight& setRadius(float radius);
        TubeLight& setIntensity(float intensity);

        vec3 position1() const;
        vec3 position2() const;
        vec3 color() const;
        float radius() const;
        float intensity() const;

        void applyModifier(const ModelState3D& state) override;
};

typedef std::shared_ptr<Light> SceneLight;
typedef std::shared_ptr<DirectionLight> SceneDirectionalLight;
typedef std::shared_ptr<PointLight> ScenePointLight;
typedef std::shared_ptr<TubeLight> SceneTubeLight;
#define newLight std::make_shared<Light>
#define newDirectionLight std::make_shared<DirectionLight>
#define newPointLight std::make_shared<PointLight>
#define newTubetLight std::make_shared<TubeLight>

class LightBuffer
{
    private :
        uint handle;
        std::shared_ptr<LightInfos[]> buffer;
        int currentID = 0;

    public :

        LightBuffer();
        ~LightBuffer();
        LightBuffer& add(Light &light);
        void send();
        void activate(int location);
        void reset();
        void update();

        LightInfos* get(){return buffer.get();};
        int maxID(){return currentID;};
};

std::ostream& operator<<(std::ostream& os, const Light &l);
std::basic_ostringstream<char32_t>& operator<<(std::basic_ostringstream<char32_t>& os, const Light &l);

class ClusteredLightBuffer
{
    private : 
        uint handle = 0;
        std::shared_ptr<int[]> buffer;
        ivec3 dimention;
        int size;

    public : 

        float vFar = 5e3;
        float ivFar = 1.f/5e3;

        ClusteredLightBuffer();
        ~ClusteredLightBuffer();

        void allocate(ivec3 dim);
        void send();
        void activate(int location);
        void update();
        
        int* get(){return buffer.get();};
        const ivec3 & dim(){return dimention;};
        int gridSize(){return size;};
};

#endif