#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <string>
#include <vector>
#include <cstring>

#include "Graphics/Skeleton.hpp"
#include "Utils.hpp"
#include "VEAC/vulpineFormats.hpp"
#include "VEAC/stencilTypes.hpp"
#include "VEAC/utils.hpp"
#include "VulpineParser.hpp"

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

#include <AssetManagerUtils.hpp>
#include <AssetManager.hpp>

#include <Scripting/ScriptInstance.hpp>

#include <Constants.hpp>

#include <Globals.hpp>

#include <assimp/anim.h>
#include <assimp/config.h>
#include <assimp/matrix4x4.h>
#include <assimp/scene.h>

#ifdef _WIN32
    #include <shlwapi.h>
    #define STR_CASE_STR(str1, str2) StrStrIA(str1, str2)
#else
    #define STR_CASE_STR(str1, str2) strcasestr(str1, str2)
#endif

ModelState3D VEAC::toModelState(aiMatrix4x4 ai)
{
    return VEAC::toModelState(toGLM(ai));
}

ModelState3D VEAC::toModelState(mat4 m)
{
    ModelState3D s;
    
    s.setPosition(vec3(vec4(m*vec4(0, 0, 0, 1))));
    
    vec3 scale(
        length(vec3(m[0][0], m[1][0], m[2][0])),
        length(vec3(m[0][1], m[1][1], m[2][1])),
        length(vec3(m[0][2], m[1][2], m[2][2]))
    );

    s.setScale(scale);

    s.setQuaternion(quat(
        mat3(
            vec3(m[0])/scale, vec3(m[1])/scale, vec3(m[2])/scale
        )
    ));

    return s;
}

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

glm::quat toGLM(const aiQuaternion a)
{
    return quat(a.w, a.x, a.y, a.z);
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

        std::cout << "Color Channels : " << mesh.GetNumColorChannels()  << "\n";

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

            if(mesh.GetNumColorChannels() > 0)
            {
                /* COLORS */
                o.w |= uint(roundf(mesh.mColors[0][i].r*31));
                o.w |= uint(roundf(mesh.mColors[0][i].g*63))<<5;
                o.w |= uint(roundf(mesh.mColors[0][i].b*31))<<11;
            }
            /* METALNESS - SMOOTHNESS - EMISSIVE */
            if(mesh.GetNumColorChannels() > 1){o.w |= uint(roundf(mesh.mColors[1][i].r   ))<<16;}
            if(mesh.GetNumColorChannels() > 2){o.w |= uint(roundf(mesh.mColors[2][i].r*15))<<17;}
            if(mesh.GetNumColorChannels() > 3){o.w |= uint(roundf(mesh.mColors[3][i].r*7 ))<<21;}
            /* SUB SURFACE - ABSORBANT - BLOODYNESS - DIRTYNESS */
            if(mesh.GetNumColorChannels() > 4){o.w |= uint(roundf(mesh.mColors[4][i].r*7))<<24;}
            if(mesh.GetNumColorChannels() > 5){o.w |= uint(roundf(mesh.mColors[5][i].r  ))<<27;}
            if(mesh.GetNumColorChannels() > 6){o.w |= uint(roundf(mesh.mColors[6][i].r*3))<<28;}
            if(mesh.GetNumColorChannels() > 7){o.w |= uint(roundf(mesh.mColors[7][i].r*3))<<30;}

            if(mesh.GetNumColorChannels() == 0)
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

void VEAC::getElementMeshSkinned(aiMesh &mesh, SkeletonRef target, STENCIL_MeshInfos &infos)
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

            std::string boneName = bone.mName.C_Str();
            replace(boneName, "mixamorig:", "");

            auto elem = target->boneNamesMap.find(boneName);

            if(elem == target->boneNamesMap.end())
            {
                continue;
                // ERROR_MESSAGE(
                //     "Can't extract skinning information in mesh '" << mesh.mName.C_Str() << 
                //     "'. Because bone '" << boneName << "' doens't correspond to any bone in the target skeleton. This mesh will have broken skinning informations."
                // )
                // infos.weights = vW;
                // infos.weightsID = vWid;
                // return;
            }

            vWid[vid][off] = elem->second;

            vW[vid][off] = w;
        }
    }

    /*
        TODO : add a check to see if all meshses have at least one bone
    */

    infos.weights = vW;
    infos.weightsID = vWid;
}

std::string saveAsVulpineMesh__BASE(aiMesh &mesh, std::string folder, VEAC_EXPORT_FORMAT format, VulpineMesh_Header &h, STENCIL_MeshInfos &infos)
{
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

std::string VEAC::saveAsVulpineMesh(aiMesh &mesh, SkeletonRef target, std::string folder, VEAC_EXPORT_FORMAT format)
{
    VulpineMesh_Header h;
    STENCIL_MeshInfos infos;

    if (mesh.HasBones())
    {
        h.type = VulpineMesh_Type::ELEMENTS_SKINNED;
        getElementMesh(mesh, infos, format);
        getElementMeshSkinned(mesh, target, infos);
    }
    else
    {
        h.type = VulpineMesh_Type::ELEMENTS;
        getElementMesh(mesh, infos, format);
    }

    return saveAsVulpineMesh__BASE(mesh, folder, format, h, infos);
}

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

    return saveAsVulpineMesh__BASE(mesh, folder, format, h, infos);
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

void VEAC::optimizeSceneBones(const aiScene &scene, Stencil_BoneMap &bonesInfosMap)
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

        std::string tmp = sorted[i].node->mName.C_Str();
        char *bonename = tmp.data();

        

        // /* cut the ugly assimp flags in names
        //    + add compatibility with animations exports down the line
        // */
        // {
        //     char *c = strstr(bonename, "_$Assimp");
        //     if (c) *c = '\0';
        // }

        // /* cut the ugly mixamo flags in names
        //    + add compatibility with animations exports down the line
        // */
        // {
        //     char *c = strstr(bonename, "mixamorig:");
        //     if (c) bonename += sizeof("mixamorig");
        // }

        // sorted[i].node->mName.Set(bonename);
            
        bonesInfosMap[bonename] = sorted[i];
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

    std::vector<std::string> names(bones.size());

    for(auto &i : bonesInfosMap)
    {
        if (!i.second.id)
        {
            continue;
        }

        // std::cout << i.second.id << "\n";

        VulpineSkeleton_Bone &b = bones[i.second.id - 1];
        aiNode &n = *i.second.node;

        b.t = toGLM(n.mTransformation);
        if (n.mParent)
            b.parent = bonesInfosMap[n.mParent->mName.C_Str()].id - 1;

        for (uint c = 0; c < n.mNumChildren; c++)
            b.children[c] = bonesInfosMap[n.mChildren[c]->mName.C_Str()].id - 1;
        
        // std::cout << i.second.id-1 << "\t" << i.first << "\n";


        // names[i.second.id-1] = i.second.node->mName.C_Str();

        std::string namecopy = i.first;
        char * bonename = namecopy.data();
        /* cut the ugly assimp flags in names
           + add compatibility with animations exports down the line
        */
        {
            char *c = strstr(bonename, "_$Assimp");
            if (c) *c = '\0';
        }

        /* cut the ugly mixamo flags in names
           + add compatibility with animations exports down the line
        */
        {
            char *c = strstr(bonename, "mixamorig:");
            if (c) bonename += sizeof("mixamorig");
        }

        names[i.second.id-1] = bonename;


        auto state3D = VEAC::toModelState(n.mTransformation);
        state3D.position = sign(state3D.position)*max(vec3(0.f), abs(state3D.position)-1e-4f);

        // std::cout << i.second.node->mName.C_Str() << "\n";
        // std::cout << "\t" << state3D.position << "\n";
        // std::cout << "\t" << state3D.quaternion << "\n";
        // std::cout << "\n";

    }

    for(auto i : bones)
        if(i.parent >= 0)
            i.t = bones[i.parent].t * i.t;

    for(auto i : bones)
            i.t = inverse(i.t);

        
    VulpineTextOutputRef out(new VulpineTextOutput(1<<15));
    
    WRITE_NAME(~, out)
    // out->Tabulate();

    int cnt = 0;
    for(auto i : bones)
    {
        auto state3D = VEAC::toModelState(i.t);
        out->Entry();
        
        
        out->write("\"", 1);
        out->write(names[cnt].c_str(), names[cnt].size());
        out->write("\"", 1);

        out->write(" ", 1);
        
        out->write("*", 1);
        FastTextParser::write<uint>(cnt, out->getReadHead());
        out->write("*", 1);
        

        out->Tabulate();
            FTXTP_WRITE_ELEMENT(state3D, position);
            FTXTP_WRITE_ELEMENT(state3D, quaternion);
            FTXTP_WRITE_ELEMENT(state3D, scale);
            FTXTP_WRITE_ELEMENT(i, parent);

            out->Entry();
            WRITE_NAME(children, out)
            out->Tabulate();
            for(auto j : i.children)
            {
                if(j)
                {
                    out->Entry();
                    FastTextParser::write<int>(j, out->getReadHead());
                    out->write("\t*", 2);
                    out->write(names[j].c_str(), names[j].size());
                    out->write("*", 1);
                }
            }
            out->Break();

        out->Break();

        cnt ++;
        // state3D.position = sign(state3D.position)*max(vec3(0.f), abs(state3D.position)-1e-4f);

        // std::cout << i.second.node->mName.C_Str() << "\n";
        // std::cout << "\t" << state3D.position << "\n";
        // std::cout << "\t" << state3D.quaternion << "\n";
        // std::cout << "\n";
    }   
    out->Break();
    // out->Break();
    out->saveAs((folder + "2").c_str());


    /* Writing to file */
    aiString name(folder);

    // VulpineSkeleton_Header h;
    // h.bonesCount = bones.size();

    // std::ofstream file(name.C_Str(), std::ios::out | std::ios::binary);
    // file.write((char *)&h, sizeof(VulpineSkeleton_Header));
    // file.write((char *)bones.data(), sizeof(VulpineSkeleton_Bone) * h.bonesCount);

    // file.flush();
    // file.close();

    // if (file.good())
    if(true)
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

int LCSubstr(const std::string &x, const std::string &y){
    int m = x.length(), n=y.length();

    int LCSuff[m][n];

    for(int j=0; j<=n; j++)
        LCSuff[0][j] = 0;
    for(int i=0; i<=m; i++)
        LCSuff[i][0] = 0;

    for(int i=1; i<=m; i++){
        for(int j=1; j<=n; j++){
            if(x[i-1] == y[j-1])
                LCSuff[i][j] = LCSuff[i-1][j-1] + 1;
            else
                LCSuff[i][j] = 0;
        }
    }

    std::string longest = "";
    for(int i=1; i<=m; i++){
        for(int j=1; j<=n; j++){
            if(LCSuff[i][j] > longest.length())
                longest = x.substr((i-LCSuff[i][j]+1) -1, LCSuff[i][j]);
        }
    }

    // std::cout << "\t" << longest;

    return longest.length();
}

std::string Longest_Common_Substring(const std::string & Str1, const std::string & Str2) {
  std::string result_substring = "";
  int m = Str1.length();
  int n = Str2.length();
  int DP_Matrix[m + 1][n + 1];
  int len = 0;
  int end = 0;

  for (int i = 0; i <= m; i++) {
    for (int j = 0; j <= n; j++) {
      if (Str1[i] == Str2[j]) {
        if ((i == 0) || (j == 0))
          DP_Matrix[i][j] = 1;
        else
          DP_Matrix[i][j] = 1 + DP_Matrix[i - 1][j - 1];

        if (DP_Matrix[i][j] > len) {
          len = DP_Matrix[i][j];
          int start = i - DP_Matrix[i][j] + 1;
          if (end == start) {
            result_substring.push_back(Str1[i]);
          } else {
            end = start;
            result_substring.clear();
            result_substring.append(Str1.substr(end, (i + 1) - end));
          }
        }
      } else {
        DP_Matrix[i][j] = 0;
      }
    }
  }
  return result_substring;
}

struct reboneMatch
{
    int originalId = -1;
    int bestMatch = -1;
    float score = 0.;

    std::string originalName;
    std::string originalNameProcessed;
};

reboneMatch findBestMatch(const std::string &name, const std::vector<std::string> & list)
{
    int maxScore = 3;
    int bestMatch = -1;
    std::string bestLCS;
    int size = list.size();

    for(int i = 0; i < size; i++)
    {
        int score = 0;

        // score += LCSubstr(name, list[i]);
        std::string LCS = Longest_Common_Substring(name, list[i]);
        score += LCS.length();
        
        // std::cout << "\t" << score << "\t" << LCS << "\t" << list[i] << "\n";

        if(score > maxScore)
        {
            maxScore = score;
            bestMatch = i;
            bestLCS = LCS;
        }

    }

    float scoreQuality = 0.f;

    // scoreQuality = ((float)bestLCS.length()) / ((float)list[bestMatch].length());

    if(bestMatch < 0)
    {
        reboneMatch result;
        result.bestMatch = -1;
        result.score = 0.f;
        return result;
    }
    
    scoreQuality = ((float)bestLCS.length()) / ((float)max(name.length(), list[bestMatch].length()));

    scoreQuality = min(scoreQuality, 1.f);

    // if(scoreQuality >= 1.f)
    //     std::cout << TERMINAL_OK;
    // else
    // if(scoreQuality > 0.5)
    //     std::cout << TERMINAL_WARNING;
    // else
    //     std::cout << TERMINAL_ERROR;

    // std::cout << "\t'" << list[bestMatch] << "'\n";
    // std::cout << "\t'" << bestLCS << "'\t" << scoreQuality  << "\n";
    // std::cout << TERMINAL_RESET;

    reboneMatch result;
    result.bestMatch = bestMatch;
    result.score = scoreQuality;

    return result;
}

void VEAC::retargetVulpineAnimation(
    const aiAnimation &anim, 
    Stencil_BoneMap &bonesInfosMap, 
    std::string filename,  
    std::string dirname, 
    std::string skeletonName, 
    std::string retargetMethodeName)
{
    float tps = anim.mTicksPerSecond > 0 ? anim.mTicksPerSecond : 30.0f;
    float durationSeconds = anim.mDuration / tps;

    SkeletonRef skeleton = Loader<SkeletonRef>::get(skeletonName);

    assert(skeleton);

    std::vector<std::string> targetBonesNames = skeleton->boneNames;
    for(auto &s : targetBonesNames)
    {
        Loader<ScriptInstance>::get("(Retarget PreProcess) " + retargetMethodeName).run(s);
        s = threadState["RETURN_string_1"];
    }

    std::vector<reboneMatch> rebones;

    std::vector<std::vector<reboneMatch>> matches(skeleton->size());

    int matchedBonesCounter = 0;
    for (unsigned int i = 0; i < anim.mNumChannels; i++)
    {
        aiNodeAnim *nodea = anim.mChannels[i];

        std::string boneName = nodea->mNodeName.C_Str();
        Stencil_BoneInfos &bone = bonesInfosMap[boneName];

        Loader<ScriptInstance>::get("(Retarget PreProcess) " + retargetMethodeName).run(boneName);
        boneName = threadState["RETURN_string_1"];

        auto match = findBestMatch(boneName, targetBonesNames);

        match.originalId = i;
        rebones.push_back(match);
        match.originalName = nodea->mNodeName.C_Str();
        match.originalNameProcessed = boneName;

        if(match.bestMatch < 0)
            match.bestMatch = 0;

        if(match.bestMatch >= 0)
            matches[match.bestMatch].push_back(match);
    }

    // skeleton->initRest();
    for(auto &i : matches)
    {
        if(i.empty())
        {
            reboneMatch tmp;
            tmp.score = 0;
            tmp.bestMatch = 0;
            tmp.originalId = 0;
            i.push_back(tmp);
        }

        matchedBonesCounter ++;
    }


    /*
        Actually Writing The File
    */

    for(char &c : filename)
        switch (c)
        {
            case '|' :
            case '@' :
                c = '_';
                break;
            
            default:break;
        }

    filename = dirname + "(" + skeletonName + ") " + filename + ".vAnimation";

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

    AnimationFileHeader head(anim.mName.C_Str(), durationSeconds, bonesInfosMap.size(), matchedBonesCounter);
    head.magicNumber = 0x494e4156;
    fwrite(&head, sizeof(AnimationFileHeader), 1, file);

    int keyframeNumber = 0;
    uint cnt = 0;
    for(auto &i : matches)
    {
        // std::string boneName = skeleton->boneNames[cnt];
        // std::cout << TERMINAL_TIMER << boneName << TERMINAL_RESET;
        // std::cout << " aka " << targetBonesNames[cnt] << "\n";

        /* Print loop */
        // for(auto &j : i)
        // {
        //     if(j.score >= 1.f)
        //         std::cout << TERMINAL_OK;
        //     else if(j.score >= .5f)
        //         std::cout << TERMINAL_WARNING;
        //     else
        //         std::cout << TERMINAL_ERROR;

        //     std::cout << "\t" << j.originalName << " aka " << j.originalNameProcessed << "\t" << j.score << "\n";
            
        //     aiNodeAnim *nodea = anim.mChannels[j.originalId];

        //     std::cout 
        //     << "\t\t" << nodea->mNumPositionKeys 
        //     << "\t" << (float)nodea->mPositionKeys[0].mTime / tps 
        //     << "\t" << (float)nodea->mPositionKeys[nodea->mNumPositionKeys-1].mTime / tps;

        //     std::cout << TERMINAL_RESET << "\n\n";
        // }


        if(i.empty())
        {
            cnt++; continue;
        }

        std::reverse(i.begin(), i.end());

        /* find max length loop */
        uint size = 0;
        bool noGoodCandidate = true;
        for(auto &j : i) if(j.score >= 1.f)
        {
            noGoodCandidate = false;
            aiNodeAnim *nodea = anim.mChannels[j.originalId];
            assert((nodea->mNumPositionKeys == nodea->mNumRotationKeys) && (nodea->mNumPositionKeys == nodea->mNumScalingKeys) && "Number of keys must be the same for position, rotation and scaling");
            size = max(size, nodea->mNumPositionKeys);
        }

        std::vector<AnimationKeyframeData> combinedAnimations(size);

        // std::cout << size << "\t" << noGoodCandidate << "\n";

        /* Combine Animations */
        aiNodeAnim *nodea_tmp = anim.mChannels[i.front().originalId];
        for(auto &j : i) if(j.score >= 1.f){nodea_tmp = anim.mChannels[j.originalId]; continue;}

        AnimationBoneData data{cnt, (AnimationBehaviour)nodea_tmp->mPreState, (AnimationBehaviour)nodea_tmp->mPostState, size};
        fwrite(&data, sizeof(AnimationBoneData), 1, file);

        std::vector<mat4> combinedTransforms(size);
        for(auto &j : combinedTransforms) j = mat4(1);

        for(auto &j : i) if(j.score >= 1.f)
        {
            aiNodeAnim *nodea = anim.mChannels[j.originalId];
            uint curSize = nodea->mNumPositionKeys;

            if(size == 1)
            {
                ModelState3D tmpState;

                tmpState
                    .setPosition(toGLM(nodea->mPositionKeys[0].mValue))
                    .setQuaternion(toGLM(nodea->mRotationKeys[0].mValue))
                    .setScale(toGLM(nodea->mScalingKeys[0].mValue))
                    .update()
                ;

                combinedTransforms[0] *= tmpState.modelMatrix;

                combinedAnimations[0].rotation = tmpState.quaternion;
                combinedAnimations[0].translation = tmpState.position;
                combinedAnimations[0].scale = tmpState.scale;
            }

            for(uint k = 0; k < size; k++)
            {
                float a = float(k) / float(size);
                float maxTime = (float)nodea->mPositionKeys[curSize-1].mTime;
                // int closestID = round(float(curSize) * a);
                int closestID = 0;

                for(uint l = 0; l < curSize; l++)
                {
                    float time = (float)nodea->mPositionKeys[l].mTime / maxTime;
                    if(time > a)
                    {
                        closestID = l-1;
                        break;
                    }
                }

                vec3 pos, scale;
                quat q;

                float lTime = (float)nodea->mPositionKeys[(closestID)%curSize].mTime / maxTime;
                float uTime = (float)nodea->mPositionKeys[(closestID+1)%curSize].mTime / maxTime;
                float interpA = (a-lTime)/(uTime-lTime);

                ModelState3D tmpState;
                tmpState
                    .setPosition(mix(
                        toGLM(nodea->mPositionKeys[closestID].mValue),
                        toGLM(nodea->mPositionKeys[closestID+1].mValue),
                        interpA
                        )
                    )
                    .setScale(mix(
                        toGLM(nodea->mScalingKeys[closestID].mValue),
                        toGLM(nodea->mScalingKeys[closestID+1].mValue),
                        interpA
                        )
                    )
                    .setQuaternion(slerp(
                        toGLM(nodea->mRotationKeys[closestID].mValue),
                        toGLM(nodea->mRotationKeys[closestID+1].mValue),
                        interpA
                        )
                    )
                    .update();
                
                combinedTransforms[k] *= tmpState.modelMatrix;
            }
        }

        for(int j = 0; j < size; j++)
        {
            mat4 matrix = combinedTransforms[j];
            mat4 skeletonMatrix;

            auto &b = skeleton->at(cnt);
            skeletonMatrix = inverse(b.t);
            if(b.parent >= 0)
                skeletonMatrix = skeleton->at(b.parent).t * inverse(b.t);

            if(noGoodCandidate) matrix = skeletonMatrix;

            ModelState3D tmpState = toModelState(matrix);

            if(size != 1 && !noGoodCandidate)
            {
                combinedAnimations[j].translation = tmpState.position;
                combinedAnimations[j].rotation = tmpState.quaternion;
                combinedAnimations[j].scale = tmpState.scale;
                combinedAnimations[j].time = (float)durationSeconds * (float)j / (float)(size-1);
            }
            
            if(noGoodCandidate)
            {
                combinedAnimations[j].time = (float)j / (float)(size-1);
            }
            
            ModelState3D sourceBone = toModelState(bonesInfosMap[nodea_tmp->mNodeName.C_Str()].node->mTransformation);
            ModelState3D destBone = toModelState(skeletonMatrix);

            Loader<ScriptInstance>::get("(Retarget) " + retargetMethodeName).run(
                std::string(nodea_tmp->mNodeName.C_Str()),
                sourceBone.quaternion,
                sourceBone.position,
                i.front().originalId,
                combinedAnimations[j].rotation,
                combinedAnimations[j].translation,
                cnt,
                skeleton->boneNames[cnt],
                destBone.quaternion,
                destBone.position,
                skeleton->at(cnt).parent,
                globals.appTime
            );

            combinedAnimations[j].rotation    = threadState["RETURN_quat_1"];
            combinedAnimations[j].translation = threadState["RETURN_vec3_1"];
            
            keyframeNumber++;
        }

        fwrite(combinedAnimations.data(), sizeof(AnimationKeyframeData), size, file);

        cnt++;
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

    return;
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
    AnimationFileHeader head(anim.mName.C_Str(), durationSeconds, bonesInfosMap.size(), anim.mNumChannels);
    head.magicNumber = 0x494e4156;

    /* Removing armature */
    for (unsigned int i = 0; i < anim.mNumChannels; i++)
    {
        aiNodeAnim *nodea = anim.mChannels[i];
        Stencil_BoneInfos &bone = bonesInfosMap[nodea->mNodeName.C_Str()];
        if(bone.id <= 0)
            head.animatedBoneNumber --;
    }

    fwrite(&head, sizeof(AnimationFileHeader), 1, file);

    int keyframeNumber = 0;
    for (unsigned int i = 0; i < anim.mNumChannels; i++)
    {
        aiNodeAnim *nodea = anim.mChannels[i];
        Stencil_BoneInfos &bone = bonesInfosMap[nodea->mNodeName.C_Str()];

        /* Removing armature */
        if(bone.id <= 0) continue;

        // std::cout << bone.id-1 << " " << nodea->mNodeName.C_Str() << "\n";

        assert((nodea->mNumPositionKeys == nodea->mNumRotationKeys) && (nodea->mNumPositionKeys == nodea->mNumScalingKeys) && "Number of keys must be the same for position, rotation and scaling");
        AnimationBoneData data{(unsigned int)bone.id-1, (AnimationBehaviour)nodea->mPreState, (AnimationBehaviour)nodea->mPostState, nodea->mNumPositionKeys};
        fwrite(&data, sizeof(AnimationBoneData), 1, file);
        for (unsigned int j = 0; j < nodea->mNumPositionKeys; j++)
        {
            quat q = quat(nodea->mRotationKeys[j].mValue.w, nodea->mRotationKeys[j].mValue.x, nodea->mRotationKeys[j].mValue.y, nodea->mRotationKeys[j].mValue.z);

            vec3 translation = vec3(nodea->mPositionKeys[j].mValue.x, nodea->mPositionKeys[j].mValue.y, nodea->mPositionKeys[j].mValue.z);

            vec3 scale = vec3(nodea->mScalingKeys[j].mValue.x, nodea->mScalingKeys[j].mValue.y, nodea->mScalingKeys[j].mValue.z);

            AnimationKeyframeData keyframe{
                durationSeconds * ((float)nodea->mPositionKeys[j].mTime / ((float)nodea->mPositionKeys[nodea->mNumPositionKeys-1].mTime))
                , 
                translation, q, scale};

            // std::cout <<  keyframe.time << "\n";

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
    VEAC::optimizeSceneBones(*scene, bonesInfosMap);

    if (bonesInfosMap.size() > 1)
        VEAC::saveAsVulpineSkeleton(bonesInfosMap, dirName + "/" + filename);

    for (unsigned int i = 0; i < scene->mNumAnimations; i++)
    {
        aiAnimation &anim = *scene->mAnimations[i];
        VEAC::saveAsVulpineAnimation(anim, bonesInfosMap, dirName + "/" + filename + "_" + anim.mName.C_Str() + ".vAnimation");
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
    const std::string &skeletonTarget,
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