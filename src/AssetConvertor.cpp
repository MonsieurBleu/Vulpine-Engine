#include <assimp/config.h>
#include <assimp/scene.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <cstring>

#include "Utils.hpp"
#include "VEAC/vulpineFormats.hpp"
#include "VEAC/stencilTypes.hpp"
#include "VEAC/utils.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags

// #define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <VEAC/AssetConvertor.hpp>

#ifdef _WIN32
    #include <shlwapi.h>
    #define STR_CASE_STR(str1, str2) StrStrIA(str1, str2)
#else
    #define STR_CASE_STR(str1, str2) strcasestr(str1, str2)
#endif

glm::mat4 toGLM(const aiMatrix4x4& from)
{
    glm::mat4 to;
    //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

glm::vec3 toGLM(const aiVector3D a)
{
    return vec3(a.x, a.y, a.z);
}

void VEAC::getElementMesh(aiMesh &mesh, STENCIL_BaseMeshInfos &infos, VEAC_EXPORT_FORMAT format)
{
    infos.facesCount = mesh.mNumFaces;
    infos.verticesCount = mesh.mNumVertices;
    infos.faces = new ivec3[mesh.mNumFaces];
    for (unsigned int i = 0; i < mesh.mNumFaces; i++)
    {
        aiFace &face = mesh.mFaces[i];
        infos.faces[i] = ivec3(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
    }
    


    if(format == FORMAT_DEFAULT)
    {
        infos.positions = (vec3 *)mesh.mVertices;
        infos.normals = (vec3 *)mesh.mNormals;
        infos.uvs = new vec2[mesh.mNumVertices];

        for (unsigned int i = 0; i < mesh.mNumVertices; i++)
            infos.uvs[i] = vec2(mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y);
    }

    if(format == FORMAT_SANCTIA)
    {
        // assert(mesh.GetNumColorChannels() == 8);

        vec3 *positions = (vec3 *)mesh.mVertices;
        vec3 *normals = (vec3 *)mesh.mNormals;
        infos.packedinfos.resize(infos.verticesCount);

        for(uint i = 0; i < infos.verticesCount; i++)
        {
            uvec4 &o = infos.packedinfos[i];
            o = uvec4(0);

            /* POSITION */
            o |= uvec4(ivec4(round(positions[i]*1e3f), 0) + 0x800000);

            float maxdot = 0.f;
            uvec3 cubeN(0.f);

            for(float scale = 0; scale < 127.5; scale+= 1.0)
            {
                vec3 t = round(normals[i]*scale);
                vec3 n = normalize(t);
                float d = dot(normals[i], n);

                if(d > maxdot)
                {
                    maxdot = d;
                    cubeN = abs(t);
                }
            }

            cubeN |= uvec3(max(sign(normals[i])*-128.f, vec3(0)));

            o |= uvec4(cubeN.x<<24, cubeN.y<<24, cubeN.z<<24, 0);
            o.w = 0;

            if(mesh.GetNumColorChannels() == 8)
            {
                /* COLORS */
                o.w |= uint(roundf(mesh.mColors[0][i].r*31));
                o.w |= uint(roundf(mesh.mColors[0][i].g*63))<<5;
                o.w |= uint(roundf(mesh.mColors[0][i].b*31))<<11;
    
                /* METALNESS - SMOOTHNESS - EMISSIVE */
                o.w |= uint(roundf(mesh.mColors[1][i].r   ))<<16;
                o.w |= uint(roundf(mesh.mColors[2][i].r*15))<<17;
                o.w |= uint(roundf(mesh.mColors[3][i].r*7 ))<<21;
    
                /* STREAKNESS - PAPERNESS - BLOODYNESS - DIRTYNESS */
                o.w |= uint(roundf(mesh.mColors[4][i].r*7))<<24;
                o.w |= uint(roundf(mesh.mColors[5][i].r  ))<<27;
                o.w |= uint(roundf(mesh.mColors[6][i].r*3))<<28;
                o.w |= uint(roundf(mesh.mColors[7][i].r*3))<<30;
            }
            else
            {
                /* COLORS */
                o.w |= uint(roundf(1.f*31));
                o.w |= uint(roundf(1.f*63))<<5;
                o.w |= uint(roundf(1.f*31))<<11;
            }
        }

    }
};

void VEAC::getElementMeshSkinned(aiMesh &mesh, Stencil_BoneMap &bonesInfosMap, STENCIL_MeshInfos &infos)
{
    vec4 *vW = new vec4[mesh.mNumVertices];
    ivec4 *vWid = new ivec4[mesh.mNumVertices];

    for (unsigned int i = 0; i < mesh.mNumVertices; i++)
    {
        vWid[i] = ivec4(-1);
        vW[i] = vec4(0);
    }

    for (unsigned int i = 0; i < mesh.mNumBones; i++)
    {
        aiBone &bone = *mesh.mBones[i];

        for (unsigned int j = 0; j < bone.mNumWeights; j++)
        {
            unsigned int vid = bone.mWeights[j].mVertexId;
            float w = bone.mWeights[j].mWeight;

            int off = 0;
            while (vWid[vid][off] != -1 && off <= 3) off++;

            vWid[vid][off] = bonesInfosMap[bone.mName.C_Str()].id - 1;;

            vW[vid][off] = w;
        }
    }

    infos.weights = vW;
    infos.weightsID = vWid;
};

/*
    TODO : adapt to new bone ids with the Stencil_BoneMap
*/
std::string VEAC::saveAsVulpineMesh(aiMesh &mesh, Stencil_BoneMap &bonesInfosMap, std::string folder, VEAC_EXPORT_FORMAT format)
{
    VulpineMesh_Header h;
    STENCIL_MeshInfos infos;

    if (mesh.HasBones())
    {
        h.type = VulpineMesh_Type::ELEMENTS_SKINNED;
        getElementMesh(mesh, infos, format);
        getElementMeshSkinned(mesh, bonesInfosMap, infos);
    }
    else
    {
        h.type = VulpineMesh_Type::ELEMENTS;
        getElementMesh(mesh, infos, format);
    }

    h.facesCount = infos.facesCount;
    h.VerticesCount = infos.verticesCount;

    h.AABBmax = vec3(mesh.mAABB.mMax.x, mesh.mAABB.mMax.y, mesh.mAABB.mMax.z);
    h.AABBmin = vec3(mesh.mAABB.mMin.x, mesh.mAABB.mMin.y, mesh.mAABB.mMin.z);

    if(format == FORMAT_SANCTIA) h.vertexPacking = COMPRESSED_UVEC4_PACKING;

    aiString name(folder);
    name.Append(mesh.mName.C_Str());
    name.Append(".vMesh");

    std::ofstream file(name.C_Str(), std::ios::out | std::ios::binary);

    file.write((char *)&h, sizeof(VulpineMesh_Header));
    file.write((char *)infos.faces, sizeof(ivec3) * infos.facesCount);

    if(format == FORMAT_DEFAULT)
    {
        file.write((const char *)infos.positions, sizeof(vec3) * infos.verticesCount);
        file.write((const char *)infos.normals, sizeof(vec3) * infos.verticesCount);
        file.write((const char *)infos.uvs, sizeof(vec2) * infos.verticesCount);
    }

    if(format == FORMAT_SANCTIA)
    {
        file.write((const char *)infos.packedinfos.data(), sizeof(uvec4) * infos.verticesCount);
    }

    if (mesh.HasBones())
    {
        file.write((const char *)infos.weights, sizeof(vec4) * infos.verticesCount);
        file.write((const char *)infos.weightsID, sizeof(ivec4) * infos.verticesCount);
    }

    file.flush();
    file.close();

    delete[] infos.uvs;
    delete[] infos.faces;

    if (mesh.HasBones())
    {
        delete[] infos.weights;
        delete[] infos.weightsID;
    }

    if (file.good())
    {
        std::cout << TERMINAL_OK
                  << "Done saving model "
                  << TERMINAL_FILENAME
                  << name.C_Str()
                  << TERMINAL_RESET
                  << "\n";
        
        return name.C_Str();
    }
    else
    {
        std::cout << TERMINAL_ERROR
                  << "Failed to write "
                  << TERMINAL_FILENAME
                  << name.C_Str()
                  << TERMINAL_RESET
                  << "\n";
        
        return "";
    }
}


void VEAC::saveTexture(aiTexture &texture, std::string folder)
{
    aiString name(folder.c_str());

    static int totalTexturesSaved = 0;
    totalTexturesSaved++;

    if (texture.mFilename.length)
        name.Append(getFileNameFromPath(texture.mFilename.C_Str()).c_str());
    else
        name.Append((std::to_string(totalTexturesSaved) + ".png").c_str());

    bool good = true;

    if (texture.mHeight)
    {
        name.Append(".png");

        stbi_write_png(
            name.C_Str(),
            texture.mWidth,
            texture.mHeight,
            4,
            texture.pcData,
            0);
    }
    else
    {
        std::fstream image(name.C_Str(), std::ios::out | std::ios::binary);

        image.write((char *)texture.pcData, texture.mWidth);

        image.flush();
        image.close();

        good = image.good();
    }

    if (good)
    {
        std::cout << TERMINAL_OK
                  << "Done saving texture "
                  << TERMINAL_FILENAME
                  << name.C_Str()
                  << TERMINAL_RESET
                  << "\n";
    }
    else
    {
        std::cout << TERMINAL_ERROR
                  << "Error saving texture "
                  << TERMINAL_FILENAME
                  << name.C_Str()
                  << TERMINAL_RESET
                  << "\n";
    }
}

void traverseGraphRec(aiNode &node, void (*f)(aiNode &, int), int rec = 0)
{
    f(node, rec);
    for (uint i = 0; i < node.mNumChildren; i++)
        traverseGraphRec(*node.mChildren[i], f, rec + 1);
}

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
void optimizeSceneBones(const aiScene &scene, Stencil_BoneMap &bonesInfosMap)
{
    /* Optimizing the mesh's bones to never point to a useless identidy nodes */
    for (uint m = 0; m < scene.mNumMeshes; m++)
    {
        aiMesh &mesh = *scene.mMeshes[m];
        for (uint i = 0; i < mesh.mNumBones; i++)
        {
            aiBone &b = *mesh.mBones[i];

            while (b.mNode->mTransformation.IsIdentity() && b.mNode != scene.mRootNode)
                b.mNode = b.mNode->mParent;
        }
    }

    /*
        Optimize useless nodes with identity matrices.
        The parent node will herit from all of their childrens
    */
    static bool identity;
    do
    {
        identity = false;

        traverseGraphRec(*scene.mRootNode, [](aiNode &n, int r)
                         {
            bool localIdentity = false;

            std::vector<aiNode *> heritage;
            for(uint i = 0; i < n.mNumChildren; i++)
            {
                aiNode &child = *n.mChildren[i];
                if(child.mTransformation.IsIdentity())
                {
                    for(uint j = 0; j < child.mNumChildren; j++)
                        heritage.push_back(child.mChildren[j]);

                    localIdentity = true;
                }
            }

            if(localIdentity)
            {
                aiNode **newChildrens = new aiNode*[heritage.size() + n.mNumChildren];
                uint i = 0;
                for(uint j = 0; j < n.mNumChildren; j++)
                    if(!n.mChildren[j]->mTransformation.IsIdentity())
                        newChildrens[i++] = n.mChildren[j];
                
                for(auto node : heritage)
                {
                    newChildrens[i++] = node;
                    node->mParent = &n;
                }
                
                delete [] n.mChildren;

                n.mChildren = newChildrens;
                n.mNumChildren = i;
            }

            identity |= localIdentity; });
    } while (identity);

    scene.mRootNode->mName.data[0] = '\0';

    /* Getting all nodes infos corresponding to bones*/
    static std::vector<Stencil_BoneInfos> sorted;
    sorted.clear();
    traverseGraphRec(*scene.mRootNode, [](aiNode &n, int r)
                     {
        if(n.mNumMeshes)
        {
            std::cout << "BONE AFFECTING 0 MESH " << n.mName.C_Str() << "\n";
            return;
        }

        if(n.mName.length == 0)
        {
            std::cout << "EMPTY NAMED MESH " << n.mName.C_Str() << "\n";
            return;
        }

        Stencil_BoneInfos i;
        i.node = &n;
        sorted.push_back(i); 
    });

    /* Sorting by name to avoid biases with files using the same skeleton
       should'nt be necessery for now, but it's cool.
    */
    std::sort(sorted.begin(), sorted.end(), [](Stencil_BoneInfos &a, Stencil_BoneInfos &b)
              { return strcmp(a.node->mName.C_Str(), b.node->mName.C_Str()) < 0; });

    /* populating the bones info map with sorted results*/
    for (int i = 0; i < (int)sorted.size(); i++)
    {
        sorted[i].id = i;

        /* cut the ugly assimp flags in names
           + add compatibility with animations exports down the line
        */
        char *c = strstr(sorted[i].node->mName.data, "_$Assimp");
        if (c)
            *c = '\0';

        bonesInfosMap[sorted[i].node->mName.C_Str()] = sorted[i];
    }

    /* OPTIMISATION :
            We sort the bones id so that that every layer of the graph
            hierarcy is contiguous.
            This specific bones arrangement enable cache optimized fast
            & stackless graph traversal.
    */
    uint16 *stack = new uint16[sorted.size()];
    memset(stack, 0, sizeof(uint16) * sorted.size());
    int back = 1;
    for (int i = 0; i < (int)sorted.size(); i++)
    {
        aiNode &n = *sorted[stack[i]].node;
        
        std::vector<const char*> sortedChildrens;
        for (uint c = 0; c < n.mNumChildren; c++)
            sortedChildrens.push_back(n.mChildren[c]->mName.C_Str());

        std::sort(
            sortedChildrens.begin(), sortedChildrens.end(), 
            [](const char *a, const char*b){return strcmp(a, b) < 0;}
        );

        for (uint c = 0; c < n.mNumChildren; c++)
        {
            stack[back++] = bonesInfosMap[sortedChildrens[c]].id;
            // stack[back++] = bonesInfosMap[n.mChildren[c]->mName.C_Str()].id;
        }
    }

    for (int i = 0; i < (int)sorted.size(); i++)
        bonesInfosMap[sorted[stack[i]].node->mName.C_Str()].id = i;
}

void VEAC::saveAsVulpineSkeleton(Stencil_BoneMap &bonesInfosMap, std::string folder)
{
    std::vector<VulpineSkeleton_Bone> bones;
    bones.resize(bonesInfosMap.size() - 1);

    for(auto &i : bonesInfosMap)
    {
        if (!i.second.id)
            continue;

        VulpineSkeleton_Bone &b = bones[i.second.id - 1];
        aiNode &n = *i.second.node;

        b.t = toGLM(n.mTransformation);
        if (n.mParent)
            b.parent = bonesInfosMap[n.mParent->mName.C_Str()].id - 1;

        for (uint c = 0; c < n.mNumChildren; c++)
            b.children[c] = bonesInfosMap[n.mChildren[c]->mName.C_Str()].id - 1;
        
        // std::cout << i.second.id-1 << "\t" << i.first << "\n";
    }

    for(auto i : bones)
        if(i.parent >= 0)
            i.t = bones[i.parent].t * i.t;

    for(auto i : bones)
            i.t = inverse(i.t);

    /* Writing to file */
    aiString name(folder);
    name.Append("Skeleton");
    name.Append(".vulpineSkeleton");

    VulpineSkeleton_Header h;
    h.bonesCount = bones.size();

    std::ofstream file(name.C_Str(), std::ios::out | std::ios::binary);
    file.write((char *)&h, sizeof(VulpineSkeleton_Header));
    file.write((char *)bones.data(), sizeof(VulpineSkeleton_Bone) * h.bonesCount);

    file.flush();
    file.close();

    if (file.good())
    {
        std::cout << TERMINAL_TIMER
                  << "Done saving skeleton (" << bones.size() << " bones) "
                  << TERMINAL_FILENAME
                  << name.C_Str()
                  << TERMINAL_TIMER
                  << TERMINAL_RESET
                  << "\n";
    }
    else
    {
        std::cout << TERMINAL_ERROR
                  << "Failed to write "
                  << TERMINAL_FILENAME
                  << name.C_Str()
                  << TERMINAL_RESET
                  << "\n";
    }
}

void VEAC::saveAsVulpineAnimation(const aiAnimation &anim, Stencil_BoneMap &bonesInfosMap, std::string filename)
{
    for(char &c : filename)
        switch (c)
        {
            case '|' :
            case '@' :
                c = '_';
                break;
            
            default:break;
        }

    FILE *file = fopen(filename.c_str(), "wb");
    if (!file)
    {
        std::cerr << TERMINAL_ERROR
                  << "Failed to open animation out file : "
                  << TERMINAL_FILENAME
                  << filename
                  << TERMINAL_RESET
                  << "\n";
        return;
    }

    float tps = anim.mTicksPerSecond > 0 ? anim.mTicksPerSecond : 30.0f;
    float durationSeconds = anim.mDuration / tps;
    // std::cout << "Duration (ticks) : " << anim.mDuration << "\n";
    // std::cout << "Ticks per second : " << anim.mTicksPerSecond << "\n";
    // std::cout << "Duration (seconds) : " << durationSeconds << "\n";
    AnimationFileHeader head(anim.mName.C_Str(), durationSeconds, bonesInfosMap.size(), anim.mNumChannels);
    fwrite(&head, sizeof(AnimationFileHeader), 1, file);

    int keyframeNumber = 0;
    for (unsigned int i = 0; i < anim.mNumChannels; i++)
    {
        aiNodeAnim *nodea = anim.mChannels[i];
        Stencil_BoneInfos &bone = bonesInfosMap[nodea->mNodeName.C_Str()];
        // std::cout << bone.id-1 << " " << nodea->mNodeName.C_Str() << "\n";
        assert((nodea->mNumPositionKeys == nodea->mNumRotationKeys) && (nodea->mNumPositionKeys == nodea->mNumScalingKeys) && "Number of keys must be the same for position, rotation and scaling");
        AnimationBoneData data{(unsigned int)bone.id-1, (AnimationBehaviour)nodea->mPreState, (AnimationBehaviour)nodea->mPostState, nodea->mNumPositionKeys};
        fwrite(&data, sizeof(AnimationBoneData), 1, file);
        for (unsigned int j = 0; j < nodea->mNumPositionKeys; j++)
        {
            quat q = quat(nodea->mRotationKeys[j].mValue.w, nodea->mRotationKeys[j].mValue.x, nodea->mRotationKeys[j].mValue.y, nodea->mRotationKeys[j].mValue.z);

            vec3 translation = vec3(nodea->mPositionKeys[j].mValue.x, nodea->mPositionKeys[j].mValue.y, nodea->mPositionKeys[j].mValue.z);

            vec3 scale = vec3(nodea->mScalingKeys[j].mValue.x, nodea->mScalingKeys[j].mValue.y, nodea->mScalingKeys[j].mValue.z);

            AnimationKeyframeData keyframe{(float)nodea->mPositionKeys[j].mTime / tps, translation, q, scale};
            fwrite(&keyframe, sizeof(AnimationKeyframeData), 1, file);

            keyframeNumber++;
        }
    }

    fclose(file);
    std::cout << TERMINAL_TIMER
              << "Done saving animation"
              << " (" << head.animatedBoneNumber << " animated bones, " << keyframeNumber << " keyframes total) "
              << TERMINAL_FILENAME
              << filename
              << TERMINAL_TIMER
              << TERMINAL_RESET
              << "\n";
}


int assetConvertor(int argc, char **argv)
{;
    VEAC_EXPORT_FORMAT format = FORMAT_DEFAULT;

    std::string outFolder = "out/";

    if (argc < 2)
    {
        std::cerr << "Usage : " << argv[0] << " <file.fbx>\n";
        return EXIT_FAILURE;
    }

    float gloabalScale = 1.f;

    for(int i = 2; i < argc; i++)
    {
        if(!strcmp(argv[i], "-scale"))
        {
            gloabalScale = atof(argv[++i]);
        }
        if(!strcmp(argv[i], "-SANCTIA"))
        {
            format = FORMAT_SANCTIA;
        }
    }

    std::string pFile(argv[1]);

    // if(!strcmp(getFileExtensionC(argv[1]), "fbx")) gloabalScale *= 100;

    Assimp::Importer importer;
    // importer.SetPropertyBool(AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, false);
    // importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    // importer.SetPropertyBool(AI_CONFIG_FBX_CONVERT_TO_M, true);
    // importer.SetPropertyBool(AI_CONFIG_FBX_USE_SKELETON_BONE_CONTAINER, true);
    importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, gloabalScale);

    // importer.SetPropertyBool(AI_CONFIG_IMPORT_COLLADA_IGNORE_UNIT_SIZE, true);
    const aiScene *scene = importer.ReadFile(pFile,
                                             // aiProcess_CalcTangentSpace         |
                                            aiProcess_Triangulate 
                                            // | aiProcess_JoinIdenticalVertices // break vertex color
                                            | aiProcess_SortByPType 
                                            | aiProcess_ImproveCacheLocality 
                                            | aiProcess_OptimizeGraph // break some animations 
                                            | aiProcess_OptimizeMeshes 
                                            | aiProcess_RemoveRedundantMaterials 
                                            | aiProcess_PopulateArmatureData 
                                            | aiProcess_GlobalScale 
                                            | aiProcess_LimitBoneWeights 
                                            | aiProcess_GenBoundingBoxes
                                            | aiProcess_GlobalScale

                                            // | aiProcess_MakeLeftHanded

                                            // aiProcess_ForceGenNormals          |
                                            // aiProcess_TransformUVCoords        |
                                            // aiProcess_GenSmoothNormals         |
                                            );

    if (nullptr == scene)
    {
        std::cerr << importer.GetErrorString();
        return EXIT_FAILURE;
    }

    // /* Print the whole graph with some cool infos */
    // traverseGraphRec(*scene->mRootNode, [](aiNode &n, int r){
    //     for(int i = 0; i < r; i++) std::cout << " ";
    //     if(n.mTransformation.IsIdentity())std::cout << TERMINAL_WARNING;
    //     std::cout << "(" << n.mNumChildren << ") "<< n.mName.C_Str();
    //     std::cout << TERMINAL_TIMER << to_string(toGLM(n.mTransformation));
    //     std::cout << TERMINAL_RESET << "\n";
    // });

    std::string filename = getNameOnlyFromPath(pFile.c_str());
    std::string dirName = outFolder + filename;
    std::filesystem::create_directory(dirName);

    static Stencil_BoneMap bonesInfosMap;
    optimizeSceneBones(*scene, bonesInfosMap);

    if (bonesInfosMap.size() > 1)
        VEAC::saveAsVulpineSkeleton(bonesInfosMap, dirName + "/" + filename);

    for (unsigned int i = 0; i < scene->mNumAnimations; i++)
    {
        aiAnimation &anim = *scene->mAnimations[i];
        VEAC::saveAsVulpineAnimation(anim, bonesInfosMap, dirName + "/" + filename + "_" + anim.mName.C_Str() + ".vulpineAnimation");
    }

    if (scene->mNumMeshes)
    {
        std::string meshesDir = dirName + "/meshes/";
        std::filesystem::create_directory(meshesDir);

        for (unsigned int i = 0; i < scene->mNumMeshes; i++)
        {
            aiMesh &mesh = *scene->mMeshes[i];

            std::cout << mesh.mNumVertices << "\n";
            std::cout << mesh.mNumFaces << "\n";

            VEAC::saveAsVulpineMesh(mesh, bonesInfosMap, meshesDir, format);
        }
    }

    if (scene->mNumTextures)
    {
        std::string texturesDir = dirName + "/textures/";
        std::filesystem::create_directory(texturesDir);

        for (unsigned int i = 0; i < scene->mNumTextures; i++)
            VEAC::saveTexture(*scene->mTextures[i], texturesDir);
    }

    std::cout << "done\n";

    return EXIT_SUCCESS;
}


/*
    TODO : redo and use only simple vulpine-wide features
*/
VEAC::FileConvertStatus VEAC::ConvertSceneFile(
    const std::string &path,
    const std::string &folder,
    VEAC_EXPORT_FORMAT format,
    unsigned int aiImportFlags,
    unsigned int vulpineImportFlags,
    float scale
)
{
    Assimp::Importer importer;

    // importer.SetPropertyBool(AI_CONFIG_FBX_CONVERT_TO_M, true);
    importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, scale);

    importer.SetPropertyBool(AI_CONFIG_PP_PTV_KEEP_HIERARCHY, true);

    // AI_CONFIG_
    
    const aiScene *scene = importer.ReadFile(path, aiImportFlags);

    if (nullptr == scene)
    {
        FILE_ERROR_MESSAGE(path, importer.GetErrorString())

        return VEAC::FileConvertStatus::FILE_MISSING;
    }

    // if (scene->mNumMeshes)
    // {
    //     for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    //     {
    //         aiMesh &mesh = *scene->mMeshes[i];

    //         std::cout << mesh.mName.C_Str() << "\n";
    //         std::cout << "\t" << mesh.mNumVertices << "\n";
    //         std::cout << "\t" << toGLM(mesh.mAABB.mMin) << "\n";
    //         std::cout << "\t" << toGLM(mesh.mAABB.mMax) << "\n";

            
    //     }   
    // }

    /*
        If we want to get full entities from the file, we need to do things differently

        The scene must be structured in a certain way :
            root : 
                entity_1
                    graphic
                        meshes
                    physics
                        primitives meshes
                entity 2
                    ...
    */
    if(vulpineImportFlags & 1<<VEAC::SceneConvertOption::OBJECT_AS_ENTITY)
    for(int i = 0; i < scene->mRootNode->mNumChildren; i++)
    {
        aiNode *collection = scene->mRootNode->mChildren[i];

        std::cout << collection->mName.C_Str() << "\n";
        std::cout << "\t" << collection->mNumChildren << "\n";

        for(int j = 0; j < collection->mNumChildren; j++)
        {
            aiNode *component = collection->mChildren[j];

            if(STR_CASE_STR(component->mName.C_Str(), "graphic"))
            {
                NOTIF_MESSAGE("Graphic !")
            }
            else 
            if(STR_CASE_STR(component->mName.C_Str(), "physic"))
            {
                NOTIF_MESSAGE("Physic !")
                for(int k = 0; k < component->mNumChildren; k++)
                {
                    aiNode *collider = component->mChildren[k];

                    if(STR_CASE_STR(collider->mName.C_Str(), "capsule"))
                    {
                        std::cout << "\tcapsule\n";

                        // CapsuleShape 

                    }
                    if(STR_CASE_STR(collider->mName.C_Str(), "cube"))
                    {
                        std::cout << "\tcube\n";
                    }
                    if(STR_CASE_STR(collider->mName.C_Str(), "sphere"))
                    {
                        std::cout << "\tsphere\n";
                    }
                }
            }
            else
            {
                FILE_ERROR_MESSAGE(
                    (path + ":" + collection->mName.C_Str()), 
                    "Entity Collection '" << component->mName.C_Str() << "' not recognized. "
                    << "Either something is wrong with the scene layout, or the VEAC entity export option was used by mistake."
                )
            }

        }

    }

    return VEAC::FileConvertStatus::ALL_GOOD;
}