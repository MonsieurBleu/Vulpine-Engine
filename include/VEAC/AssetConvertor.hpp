#pragma once 

#include <cstring>

#include "VEAC/vulpineFormats.hpp"
#include "VEAC/stencilTypes.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags

#include <MappedEnum.hpp>

namespace VulpineAssetConvertor
{
    /**
     * @brief Translate an assimp mesh into a stencil mesh for further conversions
     */
    void getElementMesh(aiMesh &mesh, STENCIL_BaseMeshInfos &infos, VEAC_EXPORT_FORMAT format = FORMAT_DEFAULT);

    /**
     * @brief Translate an assimp mesh's bones into a stencil bonemap for further conversions
     */
    void getElementMeshSkinned(aiMesh &mesh, Stencil_BoneMap &bonesInfosMap, STENCIL_MeshInfos &infos);

    /**
     * @brief Convert an assimp mesh to vulpine mesh and saves it inside the given folder
     */
    void saveAsVulpineMesh(aiMesh &mesh, Stencil_BoneMap &bonesInfosMap, std::string folder = "", VEAC_EXPORT_FORMAT format = FORMAT_DEFAULT);


    /**
     * @brief Save an assimp texture in png format into the given folder. 
     */
    void saveTexture(aiTexture &texture, std::string folder = "");

    /**
     * @brief Save a stencil bone map into a vulpine skeletton
     */
    void saveAsVulpineSkeleton(Stencil_BoneMap &bonesInfosMap, std::string folder = "");

    /**
     * @brief Save an assimp animation, using a bonemap in the vulpine animation format
     */
    void saveAsVulpineAnimation(const aiAnimation &anim, Stencil_BoneMap &bonesInfosMap, std::string filename);


    GENERATE_ENUM_FAST_REVERSE(FileConvertStatus,
        ALL_GOOD,
        FILE_MISSING,
        FILE_ERROR
    )

    GENERATE_ENUM_FAST_REVERSE(SceneConvertOption,
        OBJECT_AS_ENTITY,
        SCENE_AS_ENTITY,
        IGNORE_MESH,
        CREATE_TOP_PRIORITY_MOD
    )

    /**
     * @brief Save an assimp scene into vulpine format
     */
    FileConvertStatus ConvertSceneFile(
        const std::string &path,
        const std::string &folder,
        VEAC_EXPORT_FORMAT format,
        unsigned int aiImportFlags,
        unsigned int vulpineImportFlags,
        float scale = 1.f
    );
};

namespace VEAC = VulpineAssetConvertor;
