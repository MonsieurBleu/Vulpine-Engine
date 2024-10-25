#ifndef MESH_HPP
#define MESH_HPP

#include <string>
#include <Graphics/Shader.hpp>
#include <VertexBuffer.hpp>
#include <Graphics/Textures.hpp>

#define MESH_BASE_ATTRIBUTE_LOCATION_POSITION 0
#define MESH_BASE_ATTRIBUTE_LOCATION_COLOR    1
#define MESH_BASE_ATTRIBUTE_LOCATION_NORMAL   2
#define MESH_BASE_ATTRIBUTE_LOCATION_INSTANCE 3

/**
 * @brief TODO : update
 * 
 */
#define MESH_BASE_UNIFORM_LOCATION_MODEL_MATRIX 3

#define MESH_BASE_UNIFORM_LOCATION_MAP 20

// typedef std::shared_ptr<ShaderProgram> MeshMaterial;

class MeshMaterial : public std::shared_ptr<ShaderProgram>
{
    public : 
        MeshMaterial(std::shared_ptr<ShaderProgram> mat) : std::shared_ptr<ShaderProgram>(mat)
        {};

        MeshMaterial(ShaderProgram* material = NULL, ShaderProgram* depthOnlyMaterial = NULL);
        std::shared_ptr<ShaderProgram> depthOnly;
        
        ~MeshMaterial();
};


class MeshVao : public std::shared_ptr<VertexAttributeGroup>
{
    public : 
        MeshVao(){};
        MeshVao(VertexAttributeGroup *ptr);

        uint nbFaces = 0;
        GenericSharedBuffer faces;  
        bool animated = false;
};

class Mesh
{
    protected :
        MeshMaterial material;
        MeshVao vao;
        std::vector<Texture2D> maps;
        std::vector<int> mapsLocation;

    public :

        GLenum defaultMode = GL_TRIANGLES;
        bool invertFaces = false;
        bool depthWrite = true;
        bool noBackFaceCulling = false;
        float maxDrawDistSquared = INFINITY;

        Mesh(){};

        // Mesh(Mesh& mesh);

        Mesh(MeshMaterial material)
        : material(material) {};

        Mesh(MeshMaterial material, 
             MeshVao vao)
        : material(material), vao(vao) 
        {
            vao->generate();
        };

        MeshMaterial getMaterial(){return material;};
        MeshVao getVao(){return vao;};

        Mesh& setMaterial(MeshMaterial _material);
        Mesh& setVao(MeshVao _vao);
        Mesh& setMap(Texture2D texture, int location);
        Mesh& setMap(int location, Texture2D texture);
        Mesh& removeMap(int location);
        
        void bindAllMaps();
        void bindMap(int id, int location);
        void setBindlessMaps();

        virtual GLuint draw(GLenum mode = GL_TRIANGLES);
        virtual GLuint drawVAO(bool depthOnly = false);

        virtual bool cull();
};

#define MODEL_UNIFORM_START           16
#define MESH_MODEL_UNIFORM_MATRIX     MODEL_UNIFORM_START + 0
#define MESH_MODEL_UNIFORM_ROTATION   MODEL_UNIFORM_START + 1
#define MESH_MODEL_UNIFORM_SCALE      MODEL_UNIFORM_START + 2
#define MESH_MODEL_UNIFORM_POSITION   MODEL_UNIFORM_START + 3

#define ModelRef std::shared_ptr<MeshModel3D> 
#define newModel std::make_shared<MeshModel3D>

#define InstancedModelRef std::shared_ptr<InstancedMeshModel3D> 
#define newInstancedModel std::make_shared<InstancedMeshModel3D>

class FastUI_valueMenu;
class SkeletonAnimationState;

/*
    A special type of Mesh that contains additionnal 
    uniforms who will be updated at each drawcall 
    even without material activation.

    Those uniforms are automaticly used to update the
    object's position, rotation & scale.
*/
class MeshModel3D : public Mesh
{
    protected :
        void createUniforms();

        bool culled = true;

        vec4 lodHeigtTextureRange = vec4(0);
        vec4 lodHeightDispFactors = vec4(0);
        vec4 lodTessLevelDistance = vec4(0);

    public :

        MeshModel3D() : Mesh()
            {createUniforms();};

        MeshModel3D(Mesh& mesh) : Mesh(mesh)
            {createUniforms();};

        MeshModel3D(MeshMaterial material)
            : Mesh(material)
            {createUniforms();};

        MeshModel3D(MeshMaterial material, MeshVao vao)
            : Mesh(material, vao)
            {createUniforms();};

        MeshModel3D(MeshMaterial material, MeshVao vao, ModelState3D state)
            : Mesh(material, vao), state(state)
            {createUniforms();};

        ModelRef copy();

        ModelState3D state;
        ShaderUniformGroup uniforms;
        ShaderUniformGroup baseUniforms;
        SkeletonAnimationState *animation = nullptr;

        MeshModel3D& loadFromFolder(
            const std::string folderPath, 
            bool loadColorMap = true, 
            bool loadMaterialMap = true);

        MeshModel3D &loadFromFolderVulpine(
            const std::string folderPath, 
            bool loadColorMap = true, 
            bool loadMaterialMap = true);

        virtual void update();
        void setDrawMode();
        void resetDrawMode();
        virtual GLuint drawVAO(bool depthOnly = false);
        virtual bool cull();
        bool isCulled();

        void tessHeightTextureRange(vec2 min, vec2 max);
        void tessHeighFactors(float uvScale, float heightFactor);
        void tessDisplacementFactors(float uvScale, float displacementFactor);
        void tessActivate(vec2 minmaxTessLevel, vec2 minmaxDistance);

        void setMenu(FastUI_valueMenu &menu, std::u32string name);
};

class ModelInstance : public ModelState3D
{
    // friend InstancedMeshModel3D;

    // private : 

    // public : 
        // ModelInstance(bool &uil) : updateInstanceList(uil){};
        // bool &updateInstanceList;
};

class InstancedMeshModel3D : public MeshModel3D
{
    private :
        size_t createdInstance = 0;
        size_t allocatedInstance = 0;
        size_t drawnInstance = 0;
        
        std::shared_ptr<VertexAttribute> matricesBuffer;
        std::shared_ptr<ModelInstance*> instances;

    public :

        InstancedMeshModel3D(MeshMaterial material, MeshVao vao)
            : MeshModel3D(material, vao){};

        void allocate(size_t maxInstanceCount);
        /**
         * @brief Update the models Instance and send everything to the GPU.
         * 
         * @attention All of the instance's states are considered updated
         */
        void updateInstances();
        ModelInstance* createInstance();

        GLuint drawVAO(bool depthOnly = false) final;
        bool cull() final;
};

MeshVao readOBJ(const std::string filePath, bool useVertexColors = false);


#endif