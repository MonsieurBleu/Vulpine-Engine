#pragma once 

#include <Matrix.hpp>

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

#include <Graphics/Skeleton.hpp>

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
     * @brief Translate an assimp mesh's bones into a stencil bonemap for further conversions
     */
    void getElementMeshSkinned(aiMesh &mesh, SkeletonRef target, STENCIL_MeshInfos &infos);

    /**
     * @brief Convert an assimp mesh to vulpine mesh and saves it inside the given folder
     * @return Saved file name
     */
    std::string saveAsVulpineMesh(aiMesh &mesh, Stencil_BoneMap &bonesInfosMap, std::string folder = "", VEAC_EXPORT_FORMAT format = FORMAT_DEFAULT);

    /**
     * @brief Convert an assimp mesh to vulpine mesh and saves it inside the given folder
     * @return Saved file name
     */
    std::string saveAsVulpineMesh(aiMesh &mesh, SkeletonRef target, std::string folder = "", VEAC_EXPORT_FORMAT format  = FORMAT_DEFAULT);


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

    /**
     * @brief Save an assimp animation, after trying to retarget to a given skeleton
     */
    void retargetVulpineAnimation(
        const aiAnimation &anim, 
        Stencil_BoneMap &bonesInfosMap, 
        std::string filename, 
        std::string dirname, 
        std::string skeletonName, 
        std::string retargetMethodeName);


    /**
    * @brief This function will delete all nodes inside the scene's graphe
    * that contain idendity matrix transformation. This greatly helps reducing
    * the number of bones in the skeleton.
    *
    * This method is made for scenes with only meshes-empty nodes that
    * represent bones. If you have lights or other non meshes or bones element
    * this can cause undefined behaviour when exporting to vulpine formats.
    *
    * @attention If somehow the format imported by assimp contains meshes
        inside the hierarchy of the bones, you can be screwed when
        exporting it.
    */
    void optimizeSceneBones(const aiScene &scene, Stencil_BoneMap &bonesInfosMap);

    ModelState3D toModelState(mat4 m);
    ModelState3D toModelState(aiMatrix4x4 ai);

    GENERATE_ENUM_FAST_REVERSE(FileConvertStatus,
        ALL_GOOD,
        FILE_MISSING,
        FILE_ERROR
    )

    GENERATE_ENUM_FAST_REVERSE(SceneConvertOption,
        OBJECT_AS_ENTITY,
        SCENE_AS_ENTITY,
        IGNORE_MESH,
        EXPORT_ANIMATIONS,
        RETARGET_SKELETON,
        // CREATE_TOP_PRIORITY_MOD
    )

    /**
     * @brief Save an assimp scene into vulpine format
     */
    FileConvertStatus ConvertSceneFile(
        const std::string &path,
        const std::string &folder,
        const std::string &skeletonTarget,
        VEAC_EXPORT_FORMAT format,
        unsigned int aiImportFlags,
        unsigned int vulpineImportFlags,
        float scale = 1.f
    );
};

namespace VEAC = VulpineAssetConvertor;
