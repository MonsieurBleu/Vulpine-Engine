#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/gtc/type_ptr.hpp>
#include <memory>

using namespace glm;

#define MAX_LIGHT_COUNTER 0xff

enum lightTypes
{
    NO_LIGHT,
    DIRECTIONAL_LIGHT,
    POINT_LIGHT
};

class Light
{
    friend std::ostream& operator<<(std::ostream&, const Light &);
    // friend LightBuffer;

    protected :
        vec4 _position = vec4(0);
        vec4 _color = vec4(1);
        vec4 _direction = vec4(0);
        ivec4 _infos = vec4(0);
    
    public :
        const void* getAttribAddr();

};  

class DirectionLight : public Light
{
    public :

        DirectionLight();
        DirectionLight& setDirection(vec3 direction);
        DirectionLight& setColor(vec3 color);
        DirectionLight& setIntensity(float intensity);
};

class PointLight : public Light
{
    public :

        PointLight();
        PointLight& setPosition(vec3 position);
        PointLight& setColor(vec3 color);
        PointLight& setRadius(float radius);
        PointLight& setIntensity(float intensity);

        vec3 position() const;
        vec3 color() const;
        float intensity() const;
};




class LightBuffer
{
    private :

        uint handle;

        std::shared_ptr<Light[]> buffer;
        int currentID = 0;

    public :

        LightBuffer();
        LightBuffer& add(Light newLight);
        void send();
        void activate(int location);
        void reset();
        void update();
};

std::ostream& operator<<(std::ostream& os, const Light &l);

#endif