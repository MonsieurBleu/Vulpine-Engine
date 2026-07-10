#pragma once

#include <JoltIntegration/PhysicsCommons.hpp>
#include <Graphics/Mesh.hpp>

#include <Jolt/Renderer/DebugRenderer.h>

#include <deque>

namespace JoltVulpine
{
    class DebugRenderer : public JPH::DebugRenderer
    {
        private : 

        public :

            class Geometry : public JPH::RefTargetVirtual
            {
                public : 
                    virtual void			AddRef() override {};
                    virtual void			Release() override {};

                    InstancedModelRef model;
                    InstancedModelRef modelWire;
            };

            std::deque<InstancedModelRef> models;

            DebugRenderer();

            virtual void DrawLine
            (
                JPH::RVec3Arg inFrom,
                JPH::RVec3Arg inTo,
                JPH::ColorArg inColor
            ) 
            override;

            virtual void DrawTriangle
            (
                JPH::RVec3Arg inV1,
                JPH::RVec3Arg inV2,
                JPH::RVec3Arg inV3,
                JPH::ColorArg inColor,
                ECastShadow inCastShadow = ECastShadow::Off
            ) 
            override;

            virtual Batch CreateTriangleBatch
            (
                const Vertex *inVertices, 
                int inVertexCount, 
                const uint32 *inIndices, 
                int inIndexCount
            ) 
            override;

            virtual Batch CreateTriangleBatch(
                const Triangle *inTriangles, 
                int inTriangleCount
            ) 
            override;

            virtual void DrawGeometry
            (
                JPH::RMat44Arg inModelMatrix, 
                const JPH::AABox &inWorldSpaceBounds, 
                float inLODScaleSq, 
                JPH::ColorArg inModelColor, 
                const GeometryRef &inGeometry, 
                ECullMode inCullMode, 
                ECastShadow inCastShadow, 
                EDrawMode inDrawMode
            ) 
            override;

            virtual void DrawText3D
            (
                    JPH::RVec3Arg inPosition,
                    const std::string_view & inString,
                    JPH::ColorArg inColor = JPH::Color::sWhite,
                    float inHeight = 0.5f
            )
            override;
    };

    extern DebugRenderer *debugRenderer;
    extern JPH::BodyManager::DrawSettings debugRendererSettings;
    extern bool debugRendererActive;
}