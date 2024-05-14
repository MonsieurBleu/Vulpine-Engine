#include <Light.hpp>
#include <Fonts.hpp>

#include <GL/glew.h>
#include <Utils.hpp>

#include <Globals.hpp>

bool Light::freeShadowMapBinds[Light::maxShadowMap] = {
    true, true, true, true,
    true, true, true, true,
    true, true, true, true,
    true, true, true, true
    };

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

std::basic_ostringstream<char32_t>& operator<<(std::basic_ostringstream<char32_t>& os, const Light &l)
{
    os << U"Light : ";
        os << U"\n\tPosition : (";
        os << l.infos._position.x;
        os << U" ";
        os << l.infos._position.y;
        os << U" ";
        os << l.infos._position.z;
        os << U"), ";
        
        os << U"\n\tColor : (";
        os << l.infos._color.x;
        os  << U" ";
        os  << l.infos._color.y;
        os  << U" ";
        os  << l.infos._color.z;
        os  << U"), ";
        
        os  << U"\n\tDirection : (";
        os  << l.infos._direction.x;
        os  << U" ";
        os  << l.infos._direction.y;
        os  << U" ";
        os  << l.infos._direction.z;
        os  << U"), ";

        os  << U"\n\tIntensity :";
        os  << l.infos._color.a;

    return os;
}

void Light::activateShadows()
{
    if(shadowMap.getHandle() != 0) return;

    for(int i = 0; i < maxShadowMap; i++)
    {
        if(freeShadowMapBinds[i])
        {
            shadowMapBindId = i;
            freeShadowMapBinds[i] = false;
            break;
        }
    }

    if(shadowMapBindId == -1) return;

    infos._infos.b |= LIGHT_SHADOW_ACTIVATED;
    infos._infos.r = shadowMapBindId;

    shadowMap.addTexture(
        Texture2D()
            .setFilter(GL_NEAREST)
            .setResolution(cameraResolution)
            .setInternalFormat(GL_DEPTH_COMPONENT32F)
            .setFormat(GL_DEPTH_COMPONENT)
            .setPixelType(GL_FLOAT)
            .setAttachement(GL_DEPTH_ATTACHMENT)
            .generate()
    ).generate();
}

void Light::updateShadowCamera()
{
}

void Light::bindShadowMap()
{
    if(shadowMapBindId != -1)
        shadowMap.bindTexture(0, shadowMapBaseBind + shadowMapBindId);
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

LightBuffer::~LightBuffer()
{
    glDeleteBuffers(1, &handle);
}

void LightBuffer::send()
{   
    Light end;
    add(end);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightInfos)*MAX_LIGHT_COUNTER, buffer.get(), GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void LightBuffer::activate(int location)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, handle);
}

LightBuffer& LightBuffer::add(Light &light)
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
    Light end;
    add(end);
    glNamedBufferSubData(handle, 0, sizeof(LightInfos)*currentID, buffer.get());
}


ClusteredLightBuffer::ClusteredLightBuffer()
{
}

ClusteredLightBuffer::~ClusteredLightBuffer	()
{
    if(handle)
        glDeleteBuffers(1, &handle);
}

void ClusteredLightBuffer::allocate(ivec3 dim)
{
    if(!handle) glGenBuffers(1, &handle);
    dimention = dim;
    size = dim.x*dim.y*dim.z;
    buffer = std::shared_ptr<int[]>(new int[size*MAX_LIGHT_PER_CLUSTER]);
    memset(buffer.get(), -1, size*MAX_LIGHT_PER_CLUSTER*sizeof(int));
}

void ClusteredLightBuffer::send()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size*sizeof(int)*MAX_LIGHT_PER_CLUSTER, buffer.get(), GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ClusteredLightBuffer::activate(int location)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, handle);
}

void ClusteredLightBuffer::update()
{
    glNamedBufferSubData(handle, 0, size*MAX_LIGHT_PER_CLUSTER*sizeof(int), buffer.get());
}


LightInfos & Light::getInfos() 
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

void DirectionLight::updateShadowCamera()
{
    // vec3 position = shadowCamera.getPosition();

    vec3 cPos = globals.currentCamera->getPosition();
    cPos.y = 0;
    vec3 cDir = globals.currentCamera->getDirection();
    vec3 position = cPos + vec3(shadowCameraSize.y*0.25)*cDir;

    shadowCamera = Camera(ORTHOGRAPHIC);
    shadowCamera.init(0.f, shadowCameraSize.x, shadowCameraSize.y, 0.1, 10E3);
    shadowCamera.setDirection(direction());
    shadowCamera.setPosition(position-direction()*vec3(1E3));

    shadowCamera.updateProjectionViewMatrix();

    infos._rShadowMatrix = shadowCamera.getProjectionViewMatrix();
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

TubeLight::TubeLight()
{
    infos._infos.a = TUBE_LIGHT;
}

TubeLight& TubeLight::setPos(vec3 p1, vec3 p2)
{
    infos._position = vec4(p1, infos._position.a);
    infos._direction = vec4(p2, infos._direction.a);
    tmpPos1 = p1;
    tmpPos2 = p2;
    return *this;
}

TubeLight& TubeLight::setColor(vec3 color)
{
    infos._color = vec4(color, infos._color.a);
    return *this;
}

TubeLight& TubeLight::setRadius(float radius)
{
    infos._direction.a = radius;
    tmpRadius = radius;
    return *this;
}

TubeLight& TubeLight::setIntensity(float intensity)
{
    infos._color.a = intensity;
    return *this;
}

// TODO : complete
void TubeLight::applyModifier(const ModelState3D& state)
{

}

vec3 TubeLight::position1() const {return vec3(infos._position.x, infos._position.y, infos._position.z);};
vec3 TubeLight::position2() const {return vec3(infos._direction.x, infos._direction.y, infos._direction.z);};
vec3 TubeLight::color() const {return vec3(infos._color.x, infos._color.y, infos._color.z);};
float TubeLight::intensity() const {return infos._color.a;};
float TubeLight::radius() const {return infos._direction.a;};