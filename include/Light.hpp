#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <Matrix.hpp>

using namespace glm;

#define MAX_LIGHT_COUNTER 0xff

class ObjectGroup;

enum lightTypes
{
    NO_LIGHT,
    DIRECTIONAL_LIGHT,
    POINT_LIGHT
};

struct LightInfos
{
    vec4 _position = vec4(0);
    vec4 _color = vec4(1);
    vec4 _direction = vec4(0);
    ivec4 _infos = vec4(0);
};

class Light
{
    friend std::ostream& operator<<(std::ostream&, const Light &);
    // friend LightBuffer;
    friend ObjectGroup;

    protected :
        // vec4 _position = vec4(0);
        // vec4 _color = vec4(1);
        // vec4 _direction = vec4(0);
        // ivec4 _infos = vec4(0);

        LightInfos infos;
    
    public :
        const void* getAttribAddr() const;
        LightInfos getInfos() const;
        virtual void applyModifier(const ModelState3D& state);
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

        void applyModifier(const ModelState3D& state) override;
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

typedef std::shared_ptr<Light> SceneLight;
typedef std::shared_ptr<DirectionLight> SceneDirectionalLight;
typedef std::shared_ptr<PointLight> ScenePointLight;
#define newLight std::make_shared<Light>
#define newDirectionLight std::make_shared<DirectionLight>
#define newPointLight std::make_shared<PointLight>


class LightBuffer
{
    private :

        uint handle;

        std::shared_ptr<LightInfos[]> buffer;
        int currentID = 0;

    public :

        LightBuffer();
        LightBuffer& add(Light light);
        void send();
        void activate(int location);
        void reset();
        void update();
};

std::ostream& operator<<(std::ostream& os, const Light &l);

#endif