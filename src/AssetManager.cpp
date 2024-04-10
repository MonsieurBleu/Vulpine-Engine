#include <AssetManager.hpp>
#include <VulpineAssets.hpp>
#include <ObjectGroup.hpp>
#include <Globals.hpp>
#include <Constants.hpp>
#include <Utils.hpp>
#include <Skeleton.hpp>
#include <AssetManagerUtils.hpp>

/*
    TODO : adding Uniforms loading
*/
template<>
std::shared_ptr<ShaderProgram>& Loader<std::shared_ptr<ShaderProgram>>::loadFromInfos()
{
    EARLY_RETURN_IF_LOADED

    std::vector<const char*> elems;
    while (NEW_VALUE) elems.push_back(buff->read());

    std::vector<ShaderUniform> uniforms = globals.standartShaderUniform3D();

    switch (elems.size())
    {
    case 2 :
        r = std::make_shared<ShaderProgram>(std::string(elems[0]), std::string(elems[1]), uniforms);
        break;

    case 3 : 
        r = std::make_shared<ShaderProgram>(std::string(elems[0]), std::string(elems[1]), std::string(elems[2]), uniforms);
        break;

    case 4 :
        r = std::make_shared<ShaderProgram>(std::string(elems[0]), std::string(elems[1]), std::string(elems[2]), std::string(elems[3]), uniforms);
        break;

    default:
        FILE_ERROR_MESSAGE(name, "Wrong number of source files (" << elems.size() << ") expected 2, 3 or 4.\n")        
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

    if(!strcmp(ext, "vulpineMesh"))
        r = loadVulpineMesh(file); 
    else
    if(!strcmp(ext, "obj"))
        r = readOBJ(file);
    else
        FILE_ERROR_MESSAGE(name, "File extension '" << ext << "' not recognized. Expected .vulpineMesh or .obj")

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

        if(!strcmp(member, "source"))
        {
            char *file = buff->read();
            const char* ext = getFileExtensionC(file);

            if(!strcmp(ext, "ktx") || !strcmp(ext, "ktx2"))    
                r.loadFromFileKTX(file);
            else
            if( !strcmp(ext, "png") || !strcmp(ext, "jpeg") || 
                !strcmp(ext, "jpg") || !strcmp(ext, "tga")  || 
                !strcmp(ext, "bmp") || !strcmp(ext, "psd")  || 
                !strcmp(ext, "gif") || !strcmp(ext, "pic")  || 
                !strcmp(ext, "ppm") || !strcmp(ext, "pgm")) 
                r.loadFromFile(file);
            else
            if(!strcmp(ext, "hdr"))
                r.loadFromFileHDR(file);
            else
                FILE_ERROR_MESSAGE(name, "Image extension '" << ext << "' not supported by Vulpine.");
        }
        else
        if(!strcmp(member, "wrap"))
        {
            char *mode = buff->read();
            if(!strcmp(mode, "REPEAT"))
                r.setWrapMode(GL_REPEAT);
            else
            if(!strcmp(mode, "MIRRORED"))
                r.setWrapMode(GL_MIRRORED_REPEAT);
            else
            if(!strcmp(mode, "CLAMP_TO_EDGE"))
                r.setWrapMode(GL_CLAMP_TO_EDGE);
            else
            if(!strcmp(mode, "CLAMP_TO_BORDER"))
                r.setWrapMode(GL_CLAMP_TO_BORDER);
            else
                FILE_ERROR_MESSAGE(name, "Image wrap mode '" << mode << "' not recognized. Expected REPEAT, MIRRORED, CLAMP_TO_EDGE or CLAMP_TO_BORDER.");
        }
        else
        if(!strcmp(member, "filter"))
        {
            char *mode = buff->read();
            if(!strcmp(mode, "LINEAR"))
                r.setFilter(GL_LINEAR);
            else
            if(!strcmp(mode, "NEAREST"))
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

        if(!strcmp(member, "mesh"))
            r.setVao(LOAD_VALUE(MeshVao));
        else
        if(!strcmp(member, "material"))
            r.setMaterial(LOAD_VALUE(MeshMaterial));
        else
        if(!strcmp(member, "texture"))
        {
            int location = fromStr<int>(buff->read());
            r.setMap(location, LOAD_VALUE(Texture2D));
        }
        else
        if(!strcmp(member, "state"))
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

    while (NEW_VALUE)
    {
        char *member = buff->read();

        if(!strcmp(member, "groups"))
            while (NEW_VALUE)
                r.add(newObjectGroup(std::move(LOAD_VALUE(ObjectGroup))));
        else
        if(!strcmp(member, "meshes"))
            while (NEW_VALUE)
                r.add(LOAD_VALUE(MeshModel3D).copyWithSharedMesh());
        else
        if(!strcmp(member, "state"))
            LOAD_MODEL_STATE_3D(r.state)
        else
        if(!strcmp(member, "pointLights"))
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

    while (NEW_VALUE)
    {
        char *member = buff->read();

        if(!strcmp(member, "groups"))
            while (NEW_VALUE)
                r->add(LOAD_VALUE(ObjectGroupRef));
        else
        if(!strcmp(member, "meshes"))
            while (NEW_VALUE)
                r->add(newModel(LOAD_VALUE(MeshModel3D)));
        else
        if(!strcmp(member, "state"))
            LOAD_MODEL_STATE_3D(r->state)
        else
        if(!strcmp(member, "pointLights"))
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
    LOADER_ASSERT(NEW_VALUE)

    r = std::make_shared<Skeleton>();
    r->load(buff->read());

    EXIT_ROUTINE_AND_RETURN
}
