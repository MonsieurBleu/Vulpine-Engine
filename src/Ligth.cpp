#include <Light.hpp>

#include <GL/glew.h>

#include <Utils.hpp>

std::ostream& operator<<(std::ostream& os, const Light &l)
{
    os << TERMINAL_INFO <<
    "\nLight { " <<
        "\n\tPosition : (" << l.infos._position.x << " " << l.infos._position.y << " " << l.infos._position.z << "), " <<
        "\n\tColor : (" << l.infos._color.x << " " << l.infos._color.y << " " << l.infos._color.z << "), " <<
        "\n\tDirection : (" << l.infos._direction.x << " " << l.infos._direction.y << " " << l.infos._direction.z << "), " <<
        "\n\tIntensity :" << l.infos._color.a << ", " <<
    "\n}";
    return os;
}

const void* Light::getAttribAddr() const
{
    return &infos._position;
}

LightBuffer::LightBuffer()
{
    buffer = std::shared_ptr<LightInfos[]>(new LightInfos[MAX_LIGHT_COUNTER]);
    glGenBuffers(1, &handle);
}

void LightBuffer::send()
{
    add(Light());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightInfos)*MAX_LIGHT_COUNTER, buffer.get(), GL_DYNAMIC_COPY);
    
}

void LightBuffer::activate(int location)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, handle);
}

LightBuffer& LightBuffer::add(Light light)
{
    if(currentID >= MAX_LIGHT_COUNTER)
        return *this;
    
    buffer[currentID] = light.getInfos();
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
    glNamedBufferSubData(handle, 0, sizeof(LightInfos)*currentID, buffer.get());
}

LightInfos Light::getInfos() const
{
    return infos;
}

void Light::applyModifier(const ModelState3D& state)
{
}

DirectionLight::DirectionLight()
{
    infos._infos.a = DIRECTIONAL_LIGHT;
}

DirectionLight& DirectionLight::setDirection(vec3 direction)
{
    infos._direction = vec4(direction, infos._direction.a);
    tmpDirection = direction;
    return *this;
}

DirectionLight& DirectionLight::setColor(vec3 color)
{
    infos._color = vec4(color.r, color.g, color.b, infos._color.a);
    return *this;
}

DirectionLight& DirectionLight::setIntensity(float intensity)
{
    infos._color.a = intensity;
    return *this;
}

vec3 DirectionLight::direction()
{
    return infos._direction;
}

void DirectionLight::applyModifier(const ModelState3D& state)
{
    // Deducing rotation : http://facweb.cs.depaul.edu/andre/gam374/extractingTRS.pdf
    // vec3 invScale = vec3(
    //     1.0/length(vec3(state.modelMatrix[0][0], state.modelMatrix[1][0], state.modelMatrix[2][0])),
    //     1.0/length(vec3(state.modelMatrix[0][1], state.modelMatrix[1][1], state.modelMatrix[2][1])),
    //     1.0/length(vec3(state.modelMatrix[0][2], state.modelMatrix[1][2], state.modelMatrix[2][2]))
    // );

    // mat4 rotationMatrix = mat4(
    //     state.modelMatrix[0][0]*invScale.x, state.modelMatrix[0][1]*invScale.x, state.modelMatrix[0][2]*invScale.x, 0,
    //     state.modelMatrix[1][0]*invScale.y, state.modelMatrix[1][1]*invScale.y, state.modelMatrix[1][2]*invScale.y, 0,
    //     state.modelMatrix[2][0]*invScale.z, state.modelMatrix[2][1]*invScale.z, state.modelMatrix[2][2]*invScale.z, 0,
    //     0, 0, 0, 1
    // );

    // vec4 newDirection = rotationMatrix * vec4(tmpDirection, 1.0);
    // newDirection.a = infos._direction.a;
    // infos._direction = vec4(normalize(vec3(newDirection)), infos._direction.a);
}


PointLight::PointLight()
{
    infos._infos.a = POINT_LIGHT;
    infos._direction = vec4(1);
}

PointLight& PointLight::setPosition(vec3 position)
{
    infos._position = vec4(position, infos._position.a);
    tmpPosition = position;
    return *this;
}

PointLight& PointLight::setColor(vec3 color)
{
    infos._color = vec4(color, infos._color.a);
    return *this;
}

PointLight& PointLight::setRadius(float radius)
{
    infos._direction.x = radius;
    tmpRadius = radius;
    return *this;
}

PointLight& PointLight::setIntensity(float intensity)
{
    infos._color.a = intensity;
    return *this;
}

vec3 PointLight::position() const {return vec3(infos._position.x, infos._position.y, infos._position.z);};
vec3 PointLight::color() const {return vec3(infos._color.x, infos._color.y, infos._color.z);};
float PointLight::intensity() const {return infos._color.a;};
float PointLight::radius() const {return infos._direction.x;};

void PointLight::applyModifier(const ModelState3D& state)
{
    vec4 newPosition = state.modelMatrix * vec4(tmpPosition, 1.0);
    newPosition.a = infos._position.a;
    infos._position = newPosition;

    // Deducing scale : http://facweb.cs.depaul.edu/andre/gam374/extractingTRS.pdf
    vec3 colx = vec3(state.modelMatrix[0][0], state.modelMatrix[1][0], state.modelMatrix[2][0]);
    infos._direction.x = tmpRadius * glm::length(colx);
}
