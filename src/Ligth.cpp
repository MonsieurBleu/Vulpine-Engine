#include <Light.hpp>

#include <GL/glew.h>

#include <Utils.hpp>

std::ostream& operator<<(std::ostream& os, const Light &l)
{
    os << TERMINAL_INFO <<
    "\nLight { " <<
        "\n\tPosition : (" << l._position.x << " " << l._position.y << " " << l._position.z << "), " <<
        "\n\tColor : (" << l._color.x << " " << l._color.y << " " << l._color.z << "), " <<
        "\n\tDirection : (" << l._direction.x << " " << l._direction.y << " " << l._direction.z << "), " <<
        "\n\tIntensity :" << l._color.a << ", " <<
    "\n}";
    return os;
}

const void* Light::getAttribAddr()
{
    return &_position;
}

LightBuffer::LightBuffer()
{
    buffer = std::shared_ptr<Light[]>(new Light[MAX_LIGHT_COUNTER]);
    glGenBuffers(1, &handle);
}

void LightBuffer::send()
{
    add(Light());

    for(int i = 0; i < currentID; i ++)
    {
        std::cout << buffer[i];
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Light)*MAX_LIGHT_COUNTER, buffer.get(), GL_DYNAMIC_COPY);
    
}

void LightBuffer::activate(int location)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, handle);
}

LightBuffer& LightBuffer::add(Light newLight)
{
    if(currentID >= MAX_LIGHT_COUNTER)
        return *this;
    
    buffer[currentID] = newLight;
    currentID++;

    return *this;
}

void LightBuffer::reset()
{
    currentID = 0;
}

void LightBuffer::update()
{
    add(Light());
    glNamedBufferSubData(handle, 0, sizeof(Light)*currentID, buffer.get());
}

DirectionLight::DirectionLight()
{
    _infos.a = DIRECTIONAL_LIGHT;
}

DirectionLight& DirectionLight::setDirection(vec3 direction)
{
    _direction = vec4(direction, _direction.a);
    return *this;
}

DirectionLight& DirectionLight::setColor(vec3 color)
{
    _color = vec4(color, _color.a);
    return *this;
}

DirectionLight& DirectionLight::setIntensity(float intensity)
{
    _color.a = intensity;
    return *this;
}

PointLight::PointLight()
{
    _infos.a = POINT_LIGHT;
    _direction = vec4(1);
}

PointLight& PointLight::setPosition(vec3 position)
{
    _position = vec4(position, _position.a);
    return *this;
}

PointLight& PointLight::setColor(vec3 color)
{
    _color = vec4(color, _color.a);
    return *this;
}

PointLight& PointLight::setRadius(float radius)
{
    _direction.x = radius;
    return *this;
}

PointLight& PointLight::setIntensity(float intensity)
{
    _color.a = intensity;
    return *this;
}

vec3 PointLight::position() const {return vec3(_position.x, _position.y, _position.z);};
vec3 PointLight::color() const {return vec3(_color.x, _color.y, _color.z);};
float PointLight::intensity() const {return _color.a;};