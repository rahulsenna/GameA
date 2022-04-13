//
// Created by AgentOfChaos on 4/6/2022.
//

#include "physics.h"
#include "shader.h"

#define PVD_HOST "127.0.0.1"//Set this to the IP address of the system running the PhysX Visual Debugger that you want to connect to.
#define PX_RELEASE(x) \
    if (x)            \
    {                 \
        x->release(); \
        x = NULL;     \
    }

PxFoundation           *FoundationPhysX;
PxPhysics              *PhysXSDK;
PxCooking              *CookingPhysX;
PxDefaultCpuDispatcher *DispatcherPhysX;
PxScene                *ScenePhysX;
PxPvd                  *PvdPhysX;
PxMaterial             *DefaultMaterialPhysX;

PxDefaultAllocator     gAllocator;
PxDefaultErrorCallback gErrorCallback;

void InitPhysics()
{

    FoundationPhysX = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

    PvdPhysX                  = PxCreatePvd(*FoundationPhysX);
    PxPvdTransport *transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    PvdPhysX->connect(*transport, PxPvdInstrumentationFlag::eALL);

    PhysXSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *FoundationPhysX, PxTolerancesScale(), true, PvdPhysX);

    // Cooking
    CookingPhysX = PxCreateCooking(PX_PHYSICS_VERSION, *FoundationPhysX, PxCookingParams(PxTolerancesScale()));
    if (!CookingPhysX)
    {
        return;
    }

    //Cooking End

    PxSceneDesc sceneDesc(PhysXSDK->getTolerancesScale());
    sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;

    sceneDesc.gravity       = PxVec3(0.0f, -9.81f, 0.0f);
    DispatcherPhysX         = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = DispatcherPhysX;
    sceneDesc.filterShader  = PxDefaultSimulationFilterShader;
    ScenePhysX              = PhysXSDK->createScene(sceneDesc);
    ScenePhysX->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1);
    ScenePhysX->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);

    // Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_STATIC, 1);
    // Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_DYNAMIC, 1);
    // Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1);

    PxPvdSceneClient *pvdClient = ScenePhysX->getScenePvdClient();

    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    DefaultMaterialPhysX = PhysXSDK->createMaterial(0.5f, 0.5f, 0.6f);
}

void StepPhysics(r32 dt)
{
    ScenePhysX->simulate(1.0f / 30.0f);
    // ScenePhysX->simulate(dt);
    ScenePhysX->fetchResults(true);
}

void CleanupPhysics()
{
    PX_RELEASE(ScenePhysX);
    PX_RELEASE(DispatcherPhysX);
    PX_RELEASE(PhysXSDK);
    if (PvdPhysX)
    {
        PxPvdTransport *transport = PvdPhysX->getTransport();
        PvdPhysX->release();
        PvdPhysX = NULL;
        PX_RELEASE(transport);
    }
    PX_RELEASE(FoundationPhysX);
}

PX_INLINE void addForceAtPosInternal(PxRigidBody &body, const PxVec3 &force, const PxVec3 &pos, PxForceMode::Enum mode, bool wakeup)
{
    /*  if(mode == PxForceMode::eACCELERATION || mode == PxForceMode::eVELOCITY_CHANGE)
    {
        Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINwwwwwww_,
            "PxRigidBodyExt::addForce methods do not support eACCELERATION or eVELOCITY_CHANGE modes");
        return;
    }*/

    const PxTransform globalPose   = body.getGlobalPose();
    const PxVec3      centerOfMass = globalPose.transform(body.getCMassLocalPose().p);

    const PxVec3 torque = (pos - centerOfMass).cross(force);
    body.addForce(force, mode, wakeup);
    body.addTorque(torque, mode, wakeup);
}

static void addForceAtLocalPos(PxRigidBody &body, const PxVec3 &force, const PxVec3 &pos, PxForceMode::Enum mode, bool wakeup = true)
{
    //transform pos to world space
    const PxVec3 globalForcePos = body.getGlobalPose().transform(pos);

    addForceAtPosInternal(body, force, globalForcePos, mode, wakeup);
}

void defaultCCTInteraction(const PxControllerShapeHit &hit)
{
    PxRigidDynamic *actor = hit.shape->getActor()->is<PxRigidDynamic>();

    if (actor)
    {
        if (actor->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC)
            return;

        if (0)
        {
            const PxVec3 p = actor->getGlobalPose().p + hit.dir * 10.0f;

            PxShape *shape;
            actor->getShapes(&shape, 1);
            PxRaycastHit newHit;
            PxU32        n = PxShapeExt::raycast(*shape, *shape->getActor(), p, -hit.dir, 20.0f, PxHitFlag::ePOSITION, 1, &newHit);
            if (n)
            {
                // We only allow horizontal pushes. Vertical pushes when we stand on dynamic objects creates
                // useless stress on the solver. It would be possible to enable/disable vertical pushes on
                // particular objects, if the gameplay requires it.
                const PxVec3 upVector = hit.controller->getUpDirection();
                const PxF32  dp       = hit.dir.dot(upVector);
                //      shdfnd::printFormatted("%f\n", fabsf(dp));
                if (fabsf(dp) < 1e-3f)
                //      if(hit.dir.y==0.0f)
                {
                    const PxTransform globalPose = actor->getGlobalPose();
                    const PxVec3      localPos   = globalPose.transformInv(newHit.position);
                    ::addForceAtLocalPos(*actor, hit.dir * hit.length * 1000.0f, localPos, PxForceMode::eACCELERATION);
                }
            }
        }

        // We only allow horizontal pushes. Vertical pushes when we stand on dynamic objects creates
        // useless stress on the solver. It would be possible to enable/disable vertical pushes on
        // particular objects, if the gameplay requires it.
        const PxVec3 upVector = hit.controller->getUpDirection();
        const PxF32  dp       = hit.dir.dot(upVector);
        //      shdfnd::printFormatted("%f\n", fabsf(dp));
        if (fabsf(dp) < 1e-3f)
        //      if(hit.dir.y==0.0f)
        {
            const PxTransform globalPose = actor->getGlobalPose();
            const PxVec3      localPos   = globalPose.transformInv(toVec3(hit.worldPos));
            ::addForceAtLocalPos(*actor, hit.dir * hit.length * 1000.0f, localPos, PxForceMode::eACCELERATION);
        }
    }
}

void HitReportClass::onShapeHit(const PxControllerShapeHit &hit)
{
    defaultCCTInteraction(hit);
}

#define CONTACT_OFFSET         0.01f
//  #define CONTACT_OFFSET          0.1f
//  #define STEP_OFFSET             0.01f
#define STEP_OFFSET            0.05f
//  #define STEP_OFFSET             0.1f
//  #define STEP_OFFSET             0.2f

//  #define SLOPE_LIMIT             0.8f
#define SLOPE_LIMIT            0.0f
//  #define INVISIBLE_WALLS_HEIGHT  6.0f
#define INVISIBLE_WALLS_HEIGHT 0.0f
//  #define MAX_JUMP_HEIGHT         4.0f
#define MAX_JUMP_HEIGHT        0.0f

HitReportClass HitReportClassStub = {};

PxController *CreatePhysXCCT()
{
    PxControllerManager *CctManagerPhysX;
    PxController        *CCT_PhysX;

    // CCT Setup

    r32 ScaleFactor      = 10.5f;
    r32 StandingSize     = 1.0f * ScaleFactor;
    r32 CrouchingSize    = 0.25f * ScaleFactor;
    r32 ControllerRadius = 0.3f * ScaleFactor;

    ControlledActorDesc ControlledActorDesc;
    ControlledActorDesc.Type                = PxControllerShapeType::eCAPSULE;
    ControlledActorDesc.Position            = PxExtendedVec3(-10.0f, 20.0f, 0.0f);
    ControlledActorDesc.SlopeLimit          = SLOPE_LIMIT;
    ControlledActorDesc.ContactOffset       = CONTACT_OFFSET;
    ControlledActorDesc.StepOffset          = STEP_OFFSET;
    ControlledActorDesc.InvisibleWallHeight = INVISIBLE_WALLS_HEIGHT;
    ControlledActorDesc.MaxJumpHeight       = MAX_JUMP_HEIGHT;
    ControlledActorDesc.Radius              = ControllerRadius;
    ControlledActorDesc.Height              = StandingSize;
    ControlledActorDesc.CrouchHeight        = CrouchingSize;
    ControlledActorDesc.ReportCallback      = &HitReportClassStub;
    ControlledActorDesc.BehaviorCallback    = NULL;
    ControlledActorDesc.ProxyDensity        = (10.0f);
    ControlledActorDesc.ProxyScale          = (0.9f);
    ControlledActorDesc.VolumeGrowth        = (1.5f);

    const r32 CctRadius       = ControlledActorDesc.Radius;
    r32       CctHeight       = ControlledActorDesc.Height;
    r32       CctCrouchHeight = ControlledActorDesc.CrouchHeight;

    PxCapsuleControllerDesc CapsuleControllerDesc;
    CapsuleControllerDesc.height       = CctHeight;
    CapsuleControllerDesc.radius       = CctRadius;
    CapsuleControllerDesc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;

    PxControllerDesc *ControllerDesc;
    ControllerDesc                      = &CapsuleControllerDesc;
    ControllerDesc->density             = ControlledActorDesc.ProxyDensity;
    ControllerDesc->scaleCoeff          = ControlledActorDesc.ProxyScale;
    ControllerDesc->material            = DefaultMaterialPhysX;
    ControllerDesc->position            = ControlledActorDesc.Position;
    ControllerDesc->slopeLimit          = ControlledActorDesc.SlopeLimit;
    ControllerDesc->contactOffset       = ControlledActorDesc.ContactOffset;
    ControllerDesc->stepOffset          = ControlledActorDesc.StepOffset;
    ControllerDesc->invisibleWallHeight = ControlledActorDesc.InvisibleWallHeight;
    ControllerDesc->maxJumpHeight       = ControlledActorDesc.MaxJumpHeight;
    // ControllerDesc->nonWalkableMode      = PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;
    ControllerDesc->reportCallback = ControlledActorDesc.ReportCallback;
    // ControllerDesc->behaviorCallback = desc.BehaviorCallback;
    // ControllerDesc->volumeGrowth     = desc.VolumeGrowth;

    CctManagerPhysX               = PxCreateControllerManager(*ScenePhysX);
    CCT_PhysX                     = static_cast<PxBoxController *>(CctManagerPhysX->createController(*ControllerDesc));
    PxRigidDynamic *CctActorPhysX = CCT_PhysX->getActor();

    if (CctActorPhysX)
    {

        if (CctActorPhysX->getNbShapes())
        {
            PxShape *ctrlShape;
            CctActorPhysX->getShapes(&ctrlShape, 1);
            ctrlShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
        }
    }
    // CCT Setup END
    return CCT_PhysX;
}

void RenderPhysXDebug(u32 ShaderID, vec4 Color)
{
    SetMat4Uniform("model", mat4(1.f), ShaderID);
    SetVec4Uniform("Color", Color, ShaderID);

    const PxRenderBuffer &Buffer = ScenePhysX->getRenderBuffer();
    std::vector<PxVec3>   Vertices;
    for (PxU32 I = 0; I < Buffer.getNbLines(); I++)
    {
        const PxDebugLine &line = Buffer.getLines()[I];
        // render the line
        Vertices.push_back(line.pos0);
        Vertices.push_back(line.pos1);
    }

    createVAO();
    u32 VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(PxVec3), Vertices.data(), GL_DYNAMIC_DRAW);
    linkAttrib(VBO, 0, 3, GL_FLOAT, 1 * sizeof(PxVec3), (void *) 0);
    glDrawArrays(GL_LINES, 0, Vertices.size());
}

void RenderPhysXMesh(u32 VAO, u32 ShaderID, s32 IndicesSize,
                     mat4 view, mat4 projection, mat4 ModelMatrix, vec3 color)
{

    glUseProgram(ShaderID);
    SetMat4Uniform("projection", projection, ShaderID);
    SetMat4Uniform("view", view, ShaderID);

    SetVec4Uniform("Color", vec4(0, 1, 0, 1), ShaderID);
    SetMat4Uniform("model", ModelMatrix, ShaderID);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    bindVAO(VAO);

    glDrawElements(GL_TRIANGLES, IndicesSize, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

u32 SetUpPhysXMeshForRendering(PxTriangleMesh *TriangleMesh)
{
    u32 VAO = createVAO();
    u32 VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, TriangleMesh->getNbVertices() * sizeof(PxVec3), TriangleMesh->getVertices(), GL_STATIC_DRAW);

    u32 EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, TriangleMesh->getNbTriangles() * 3 * sizeof(u32), TriangleMesh->getTriangles(), GL_STATIC_DRAW);

    linkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(PxVec3), (void *) 0);

    return VAO;
}

std::vector<PxVec3> GetAllVerticesFromModel(model *Model)
{
    unsigned int MeshCount = Model->meshes.size();

    std::vector<PxVec3> Result;
    for (u32 i = 0; i < MeshCount; ++i)
    {
        unsigned int VertCount = Model->meshes[i].vertices.size();

        for (u32 j = 0; j < VertCount; ++j)
        {
            vec3 pos = Model->meshes[i].vertices[j].position;
            Result.push_back(PxVec3(pos.x, pos.y, pos.z));
        }
    }
    return Result;
}

std::vector<PxU32> GetAllIndicesFromModel(model *Model)
{
    unsigned int MeshCount = Model->meshes.size();

    std::vector<PxU32> Result;
    for (u32 i = 0; i < MeshCount; ++i)
    {
        std::vector<PxU32> Indices = Model->meshes[i].indices;
        Result.insert(Result.end(), Indices.begin(), Indices.end());
    }
    return Result;
}

#include "PsTime.h"
PxReal getElapsedTimeInMilliseconds(const PxU64 elapsedTime)
{
    return shdfnd::Time::getCounterFrequency().toTensOfNanos(elapsedTime) / (100.0f * 1000.0f);
}

// Setup common cooking params
void setupCommonCookingParams(PxCookingParams &params, bool skipMeshCleanup, bool skipEdgeData)
{
    // we suppress the triangle mesh remap table computation to gain some speed, as we will not need it
    // in this snippet
    params.suppressTriangleMeshRemapTable = true;

    // If DISABLE_CLEAN_MESH is set, the mesh is not cleaned during the cooking. The input mesh must be valid.
    // The following conditions are true for a valid triangle mesh :
    //  1. There are no duplicate vertices(within specified vertexWeldTolerance.See PxCookingParams::meshWeldTolerance)
    //  2. There are no large triangles(within specified PxTolerancesScale.)
    // It is recommended to run a separate validation check in debug/checked builds, see below.

    if (!skipMeshCleanup)
        params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH);
    else
        params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;

    // If DISABLE_ACTIVE_EDGES_PREDOCOMPUTE is set, the cooking does not compute the active (convex) edges, and instead
    // marks all edges as active. This makes cooking faster but can slow down contact generation. This flag may change
    // the collision behavior, as all edges of the triangle mesh will now be considered active.
    if (!skipEdgeData)
        params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE);
    else
        params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
}

// Creates a triangle mesh using BVH34 midphase with different settings.
PxTriangleMesh *createBV34TriangleMesh(const char *Filename,
                                       PxU32 numVertices, const PxVec3 *vertices,
                                       PxU32 numTriangles, const PxU32 *indices,
                                       bool skipMeshCleanup, bool skipEdgeData,
                                       bool inserted, const PxU32 numTrisPerLeaf)
{
    PxU64 startTime = shdfnd::Time::getCurrentCounterValue();

    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count     = numVertices;
    meshDesc.points.data      = vertices;
    meshDesc.points.stride    = sizeof(PxVec3);
    meshDesc.triangles.count  = numTriangles;
    meshDesc.triangles.data   = indices;
    meshDesc.triangles.stride = 3 * sizeof(PxU32);

    PxCookingParams params = CookingPhysX->getParams();

    // Create BVH34 midphase
    params.midphaseDesc = PxMeshMidPhase::eBVH34;

    // setup common cooking params
    setupCommonCookingParams(params, skipMeshCleanup, skipEdgeData);

    // Cooking mesh with less triangles per leaf produces larger meshes with better runtime performance
    // and worse cooking performance. Cooking time is better when more triangles per leaf are used.
    params.midphaseDesc.mBVH34Desc.numPrimsPerLeaf = numTrisPerLeaf;

    CookingPhysX->setParams(params);

#if defined(PX_CHECKED) || defined(PX_DEBUG)
    // If DISABLE_CLEAN_MESH is set, the mesh is not cleaned during the cooking.
    // We should check the validity of provided triangles in debug/checked builds though.
    if (skipMeshCleanup)
    {
        PX_ASSERT(CookingPhysX->validateTriangleMesh(meshDesc));
    }
#endif// DEBUG

    PxTriangleMesh *triMesh  = NULL;
    PxU32           meshSize = 0;

    // The cooked mesh may either be saved to a stream for later loading, or inserted directly into PxPhysics.
    if (inserted)
    {
        triMesh = CookingPhysX->createTriangleMesh(meshDesc, PhysXSDK->getPhysicsInsertionCallback());
    } else
    {
        PxDefaultMemoryOutputStream outBuffer;
        CookingPhysX->cookTriangleMesh(meshDesc, outBuffer);

        WriteToFile(Filename, outBuffer.getData(), outBuffer.getSize());

        PxDefaultMemoryInputData stream(outBuffer.getData(), outBuffer.getSize());
        triMesh = PhysXSDK->createTriangleMesh(stream);

        meshSize = outBuffer.getSize();
    }

    // Print the elapsed time for comparison
    PxU64 stopTime    = shdfnd::Time::getCurrentCounterValue();
    float elapsedTime = getElapsedTimeInMilliseconds(stopTime - startTime);
    printf("\t -----------------------------------------------\n");
    printf("\t Create triangle mesh with %d triangles: \n", numTriangles);
    inserted ? printf("\t\t Mesh inserted on\n") : printf("\t\t Mesh inserted off\n");
    !skipEdgeData ? printf("\t\t Precompute edge data on\n") : printf("\t\t Precompute edge data off\n");
    !skipMeshCleanup ? printf("\t\t Mesh cleanup on\n") : printf("\t\t Mesh cleanup off\n");
    printf("\t\t Num triangles per leaf: %d \n", numTrisPerLeaf);
    printf("\t Elapsed time in ms: %f \n", double(elapsedTime));
    if (!inserted)
    {
        printf("\t Mesh size: %d \n", meshSize);
    }

    // triMesh->release();

    return triMesh;
}

PxTriangleMesh *LoadTriangleMesh(const char *Filename)
{
    FileData data = ReadFromFile(Filename);

    PxDefaultMemoryInputData stream((PxU8 *) data.data, data.size);
    PxTriangleMesh          *triMesh = PhysXSDK->createTriangleMesh(stream);
    return triMesh;
}