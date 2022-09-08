//
// Created by AgentOfChaos on 4/6/2022.
//

#pragma once

#include "platform.h"
#include "PxPhysicsAPI.h"
#include "model.h"

using namespace physx;

struct ControlledActorDesc
{

    PxControllerShapeType::Enum   Type;
    PxExtendedVec3                Position;
    float                         SlopeLimit;
    float                         ContactOffset;
    float                         StepOffset;
    float                         InvisibleWallHeight;
    float                         MaxJumpHeight;
    float                         Radius;
    float                         Height;
    float                         CrouchHeight;
    float                         ProxyDensity;
    float                         ProxyScale;
    float                         VolumeGrowth;
    PxUserControllerHitReport    *ReportCallback;
    PxControllerBehaviorCallback *BehaviorCallback;
};

struct physx_actor_entity
{
    PxRigidActor *actorPtr;
    PxU32         actorId;
};

struct physics_engine
{
    PxFoundation           *Foundation;
    PxPhysics              *SDK;
    PxCooking              *Cooking;
    PxDefaultCpuDispatcher *Dispatcher;
    PxScene                *Scene;
    PxPvd                  *Pvd;
};

struct raycast_info
{
    b32  DidHit;
    vec3 Position;
};

void InitPhysics();

physics_engine *InitPhysicsEngine();

void StepPhysics(r32 dt);

void CleanupPhysics();

class HitReportClass : public PxUserControllerHitReport
{
    // Implements PxUserControllerHitReport
    virtual void onShapeHit(const PxControllerShapeHit &hit);

    virtual void onControllerHit(const PxControllersHit &hit)
    {}

    virtual void onObstacleHit(const PxControllerObstacleHit &hit)
    {}
};

inline vec3 fromPxExtVec3(const PxExtendedVec3 &vec)
{
    return {vec.x, vec.y, vec.z};
}

inline vec3 fromPxVec3(const PxVec3 &vec)
{
    return {vec.x, vec.y, vec.z};
}

inline PxVec3 toPxVec3(const vec3 &vec)
{
    return {vec.x, vec.y, vec.z};
}

PxController *CreatePhysXCCT();

void RenderPhysXMesh(u32 VAO, u32 ShaderID, s32 IndicesSize,
                     mat4 view, mat4 projection, mat4 ModelMatrix, vec3 color = vec3(0, 1, 0));

u32 SetUpPhysXMeshForRendering(PxTriangleMesh *TriangleMesh);

std::vector<PxVec3> GetAllVerticesFromModel(model *Model);

std::vector<PxU32> GetAllIndicesFromModel(model *Model);

PxTriangleMesh *LoadTriangleMesh(const char *Filename);

PxTriangleMesh *createBV34TriangleMesh(const char *Filename,
                                       PxU32 numVertices, const PxVec3 *vertices,
                                       PxU32 numTriangles, const PxU32 *indices,
                                       bool skipMeshCleanup, bool skipEdgeData,
                                       bool inserted, const PxU32 numTrisPerLeaf);

raycast_info RayCast(PxGeometry &geometry, PxTransform transform, PxVec3 origin, PxVec3 direction, PxReal distance);
