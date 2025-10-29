#include "Matrix.hpp"
#include "VulpineParser.hpp"
#include <AssetManager.hpp>
#include <VulpineAssets.hpp>
#include <Graphics/ObjectGroup.hpp>
#include <Globals.hpp>
#include <Constants.hpp>
#include <Utils.hpp>
#include <Graphics/Skeleton.hpp>
#include <AssetManagerUtils.hpp>

#include <Scripting/ScriptInstance.hpp>
#include <glm/ext/vector_float3.hpp>
#include <strings.h>

/*
    TODO : adding Uniforms loading
*/
template<>
std::shared_ptr<ShaderProgram>& Loader<std::shared_ptr<ShaderProgram>>::loadFromInfos()
{
    EARLY_RETURN_IF_LOADED

    bool uniformsSet = false;
    char emptypreproc[1] = "";
    char *preproc = emptypreproc;
    std::vector<ShaderUniform> uniforms;

    std::vector<const char*> elems;
    while (NEW_VALUE)
    {
        const char *elem = buff->read();

        if(!strcasecmp(elem, "uniforms"))
        {
            const char *unitype = buff->read();
            uniformsSet = true;

            if(!strcasecmp(unitype, "2D"))
                uniforms = globals.standartShaderUniform2D();
            else
            if(!strcasecmp(unitype, "3D"))
                uniforms = globals.standartShaderUniform3D();
            else
            if(!strcasecmp(unitype, "none"))
            {
                FILE_ERROR_MESSAGE(name, "Uniform preset " << unitype << " not recognized. Expected 2D, 3D or none.")
            }
        }
        else
        if(!strcasecmp(elem, "directive"))
        {
            preproc = buff->read();
        }
        else if(!strcasecmp(elem, "frag"))
        {
            std::string name(buff->read());
            elems.push_back(Loader<ShaderFragPath>::get(name).path.c_str());
        }
        else if(!strcasecmp(elem, "vert"))
        {
            std::string name(buff->read());
            elems.push_back(Loader<ShaderVertPath>::get(name).path.c_str());
        }
        else if(!strcasecmp(elem, "geom"))
        {
            std::string name(buff->read());
            elems.push_back(Loader<ShaderGeomPath>::get(name).path.c_str());
        }
        else if(!strcasecmp(elem, "tesc"))
        {
            std::string name(buff->read());
            elems.push_back(Loader<ShaderTescPath>::get(name).path.c_str());
        }
        else if(!strcasecmp(elem, "tese"))
        {
            std::string name(buff->read());
            elems.push_back(Loader<ShaderTesePath>::get(name).path.c_str());
        }
        else
            elems.push_back(elem);
    }

    if(!uniformsSet)
        uniforms = globals.standartShaderUniform3D();

    switch (elems.size())
    {
    case 2 :
        r = std::make_shared<ShaderProgram>(std::string(elems[0]), std::string(elems[1]), uniforms, std::string(preproc));
        break;

    case 3 : 
        r = std::make_shared<ShaderProgram>(std::string(elems[0]), std::string(elems[1]), std::string(elems[2]), uniforms, std::string(preproc));
        break;

    case 4 :
        r = std::make_shared<ShaderProgram>(std::string(elems[0]), std::string(elems[1]), std::string(elems[2]), std::string(elems[3]), uniforms, std::string(preproc));
        break;

    default:
        FILE_ERROR_MESSAGE(name, "Wrong number of source files (" << elems.size() << ") expected 2, 3 or 4.")        
        LOADER_ASSERT(elems.size() > 1 && elems.size() < 5)
        break;
    }


    EXIT_ROUTINE_AND_RETURN
}


template<>
MeshMaterial& Loader<MeshMaterial>::loadFromInfos()
{
    EARLY_RETURN_IF_LOADED
    LOADER_ASSERT(NEW_VALUE)

    r = LOAD_VALUE(std::shared_ptr<ShaderProgram>);

    if(NEW_VALUE)
    {
        r.depthOnly = LOAD_VALUE(std::shared_ptr<ShaderProgram>);
        LOADER_ASSERT(END_VALUE)
    }
    EXIT_ROUTINE_AND_RETURN
}


template<>
MeshVao& Loader<MeshVao>::loadFromInfos()
{
    EARLY_RETURN_IF_LOADED
    LOADER_ASSERT(NEW_VALUE)

    char *file = buff->read();
    const char* ext = getFileExtensionC(file);

    if(!strcasecmp(ext, "vMesh"))
        r = loadVulpineMesh(file); 
    else
    if(!strcasecmp(ext, "obj"))
        r = readOBJ(file);
    else
        FILE_ERROR_MESSAGE(name, "File extension '" << ext << "' not recognized. Expected .vMesh or .obj")

    LOADER_ASSERT(END_VALUE)
    EXIT_ROUTINE_AND_RETURN
}

template<>
Texture2D& Loader<Texture2D>::loadFromInfos()
{
    EARLY_RETURN_IF_LOADED

    while (NEW_VALUE)
    {
        char *member = buff->read();

        if(!strcasecmp(member, "source"))
        {
            char *file = buff->read();
            const char* ext = getFileExtensionC(file);

            if(!strcasecmp(ext, "ktx") || !strcasecmp(ext, "ktx2"))    
                r.loadFromFileKTX(file);
            else
            if( !strcasecmp(ext, "png") || !strcasecmp(ext, "jpeg") || 
                !strcasecmp(ext, "jpg") || !strcasecmp(ext, "tga")  || 
                !strcasecmp(ext, "bmp") || !strcasecmp(ext, "psd")  || 
                !strcasecmp(ext, "gif") || !strcasecmp(ext, "pic")  || 
                !strcasecmp(ext, "ppm") || !strcasecmp(ext, "pgm")) 
                r.loadFromFile(file);
            else
            if(!strcasecmp(ext, "hdr"))
                r.loadFromFileHDR(file);
            else
            if(!strcasecmp(ext, "exr"))
                r.loadFromFileEXR(file);
            else
                FILE_ERROR_MESSAGE(name, "Image extension '" << ext << "' not supported by Vulpine.");
        }
        else
        if(!strcasecmp(member, "wrap"))
        {
            char *mode = buff->read();
            if(!strcasecmp(mode, "REPEAT"))
                r.setWrapMode(GL_REPEAT);
            else
            if(!strcasecmp(mode, "MIRRORED"))
                r.setWrapMode(GL_MIRRORED_REPEAT);
            else
            if(!strcasecmp(mode, "CLAMP_TO_EDGE"))
                r.setWrapMode(GL_CLAMP_TO_EDGE);
            else
            if(!strcasecmp(mode, "CLAMP_TO_BORDER"))
                r.setWrapMode(GL_CLAMP_TO_BORDER);
            else
                FILE_ERROR_MESSAGE(name, "Image wrap mode '" << mode << "' not recognized. Expected REPEAT, MIRRORED, CLAMP_TO_EDGE or CLAMP_TO_BORDER.");
        }
        else
        if(!strcasecmp(member, "filter"))
        {
            char *mode = buff->read();
            if(!strcasecmp(mode, "LINEAR"))
                r.setFilter(GL_LINEAR);
            else
            if(!strcasecmp(mode, "NEAREST"))
                r.setFilter(GL_NEAREST);
            else
                FILE_ERROR_MESSAGE(name, "Image filter mode '" << mode << "' not recognized. Expected NEAREST or LINEAR.");            
        }
        else
            FILE_ERROR_MESSAGE(name, "Image member '" << member << "' not recognized. Expected source, wrap or filter.");
    }

    r.generate();

    EXIT_ROUTINE_AND_RETURN
}

template<>
MeshModel3D& Loader<MeshModel3D>::loadFromInfos()
{
    EARLY_RETURN_IF_LOADED

    // r = newModel();

    while (NEW_VALUE)
    {
        char *member = buff->read();

        if(!strcasecmp(member, "mesh"))
            r.setVao(LOAD_VALUE(MeshVao));
        else
        if(!strcasecmp(member, "material"))
            r.setMaterial(LOAD_VALUE(MeshMaterial));
        else
        if(!strcasecmp(member, "texture"))
        {
            int location = fromStr<int>(buff->read());
            r.setMap(location, LOAD_VALUE(Texture2D));
        }
        else
        if(!strcasecmp(member, "state"))
            LOAD_MODEL_STATE_3D(r.state)
        else
            FILE_ERROR_MESSAGE(name, "ModelRef member '" << member << "' not recognized.");
    } 

    EXIT_ROUTINE_AND_RETURN 
}


template<>
ObjectGroup& Loader<ObjectGroup>::loadFromInfos()
{
    EARLY_RETURN_IF_LOADED

    r.name = std::string(name);

    while (NEW_VALUE)
    {
        char *member = buff->read();

        if(!strcasecmp(member, "groups"))
            while (NEW_VALUE)
                r.add(newObjectGroup(std::move(LOAD_VALUE(ObjectGroup))));
        else
        if(!strcasecmp(member, "meshes"))
            while (NEW_VALUE)
                r.add(LOAD_VALUE(MeshModel3D).copy());
        else
        if(!strcasecmp(member, "state"))
            LOAD_MODEL_STATE_3D(r.state)
        else
        if(!strcasecmp(member, "pointLights"))
            while (NEW_VALUE)
            {
                ScenePointLight l = newPointLight();

                unsigned int colorI = strtol(buff->read(), nullptr, 16);
                l->setColor(vec3((colorI&0xFF0000)>>16, (colorI&0x00FF00)>>8, colorI&0x0000FF)/255.f)
                    .setIntensity(fromStr<float>(buff->read()))
                    .setRadius(fromStr<float>(buff->read()))
                    .setPosition(fromStr<vec3>(buff->read()));
                    
                r.add(l);
            }
        else
            FILE_ERROR_MESSAGE(name, "ObjectGroupRef member '" << member << "' not recognized.");
    }

    EXIT_ROUTINE_AND_RETURN 
}

template<>
ObjectGroupRef& Loader<ObjectGroupRef>::loadFromInfos()
{
    EARLY_RETURN_IF_LOADED

    r = newObjectGroup();
    r->name = std::string(name);

    while (NEW_VALUE)
    {
        char *member = buff->read();

        if(!strcasecmp(member, "groups"))
            while (NEW_VALUE)
                r->add(LOAD_VALUE(ObjectGroupRef));
        else
        if(!strcasecmp(member, "meshes"))
            while (NEW_VALUE)
                r->add(newModel(LOAD_VALUE(MeshModel3D)));
        else
        if(!strcasecmp(member, "state"))
            LOAD_MODEL_STATE_3D(r->state)
        else
        if(!strcasecmp(member, "pointLights"))
            while (NEW_VALUE)
            {
                ScenePointLight l = newPointLight();

                unsigned int colorI = strtol(buff->read(), nullptr, 16);
                l->setColor(vec3((colorI&0xFF0000)>>16, (colorI&0x00FF00)>>8, colorI&0x0000FF)/255.f)
                    .setIntensity(fromStr<float>(buff->read()))
                    .setRadius(fromStr<float>(buff->read()))
                    .setPosition(fromStr<vec3>(buff->read()));
                    
                r->add(l);
            }
        else
            FILE_ERROR_MESSAGE(name, "ObjectGroupRef member '" << member << "' not recognized.");
    }

    EXIT_ROUTINE_AND_RETURN 
}

template<>
SkeletonRef& Loader<SkeletonRef>::loadFromInfos()
{
    EARLY_RETURN_IF_LOADED

    // LOADER_ASSERT(NEW_VALUE)

    r = std::make_shared<Skeleton>();

    while (NEW_VALUE)
    {
        char *member = buff->read();

        if(!strcasecmp(member, "bin"))
        {
            r->load(buff->read());
        }
        else
        {
            SkeletonBone b;

            ModelState3D tmp;

            while (NEW_VALUE)
            {
                char *submember = buff->read();
                const char *value = nullptr;

                if(!strcasecmp(submember, "position"))
                {
                    value = buff->read();
                    tmp.setPosition(FastTextParser::read<vec3>(value));
                }
                else
                if(!strcasecmp(submember, "quaternion"))
                {
                    value = buff->read();
                    tmp.setQuaternion(FastTextParser::read<quat>(value));
                }
                else
                if(!strcasecmp(submember, "scale"))
                {
                    value = buff->read();
                    tmp.setScale(FastTextParser::read<vec3>(value));
                }
                else
                if(!strcasecmp(submember, "parent"))
                {
                    value = buff->read();
                    b.parent = FastTextParser::read<int>(value);
                }
                else
                if(!strcasecmp(submember, "children"))
                {
                    int cnt = 0;

                    while (NEW_VALUE)
                    {
                        value = buff->read();
                        b.children[cnt] = FastTextParser::read<int>(value);

                        if(value > buff->data + buff->getReadHead())
                            buff->setHead(value);
                    }
                }
                else
                    FILE_ERROR_MESSAGE(name, "SkeletonBone member '" << submember << "' not recognized.");

                if(value > buff->data + buff->getReadHead())
                    buff->setHead(value);
                
            }

            tmp.update();
            b.t = tmp.modelMatrix;
            r->boneNames.push_back(member);
            r->boneNamesMap[member] = r->boneNames.size()-1;
            r->push_back(b);
        }
    }

    r->initRest();
    r->name = name;

    EXIT_ROUTINE_AND_RETURN
}

template<>
AnimationRef& Loader<AnimationRef>::loadFromInfos()
{
    EARLY_RETURN_IF_LOADED
    LOADER_ASSERT(NEW_VALUE)
    
    r = Animation::load(buff->read());

    EXIT_ROUTINE_AND_RETURN
}

template<>
ScriptInstance& Loader<ScriptInstance>::loadFromInfos()
{
    EARLY_RETURN_IF_LOADED
    LOADER_ASSERT(NEW_VALUE)

    r = ScriptInstance(buff->read());

    EXIT_ROUTINE_AND_RETURN
}

#define GEN_SHADER_PATH_LOADER(type) template<>type& Loader<type>::loadFromInfos() \
    { EARLY_RETURN_IF_LOADED LOADER_ASSERT(NEW_VALUE) r.path = std::string(buff->read()); EXIT_ROUTINE_AND_RETURN}

GEN_SHADER_PATH_LOADER(ShaderFragPath);
GEN_SHADER_PATH_LOADER(ShaderVertPath);
GEN_SHADER_PATH_LOADER(ShaderGeomPath);
GEN_SHADER_PATH_LOADER(ShaderTescPath);
GEN_SHADER_PATH_LOADER(ShaderTesePath);
GEN_SHADER_PATH_LOADER(ShaderInclPath);
