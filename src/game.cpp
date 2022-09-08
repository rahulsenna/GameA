//
// Created by AgentOfChaos on 4/5/2022.
//

#include "game.h"

#include "glm/gtx/string_cast.hpp"
#include "shader.h"
#include "basic_shapes.h"
#include "camera.h"

#include "physics.h"

#include "render_group.h"
#include "input.h"
#include "model.h"
#include "animation.h"

camera Camera;
player Player = {};

PxTransform            PhysXTerrainTransform;
PxTriangleMeshGeometry PhysXTriangleGeom;

// Extern Variables
extern PxFoundation           *FoundationPhysX;
extern PxPhysics              *PhysXSDK;
extern PxCooking              *CookingPhysX;
extern PxDefaultCpuDispatcher *DispatcherPhysX;
extern PxScene                *ScenePhysX;
extern PxPvd                  *PvdPhysX;
extern PxMaterial             *DefaultMaterialPhysX;
// Extern Variables END

render_group_shape         ShapeRenderGroup;
dynamic_model_render_group DynamicModelRenderGroup;
static_model_render_group  StaticModelRenderGroup;

PxController *MainCCT;

shadow_map ShadowMap;

u32 DebugLineShader;

debug_camera DebugCamera;

ImGuiIO *ImGuiIo = nullptr;

void AddContent();

bloom_properties Bloom;

water Water;

cubemap Skybox;
void    InitGame(GLFWwindow *Window)
{
    Skybox = CreateCubemap();
    Camera = InitCamera(glm::vec3(0, 2, 4));

    DebugCamera.Enable          = false;
    DebugCamera.EnableShadowBox = false;
    // DebugCamera.Pos             = vec3(500, 200, -100);
    DebugCamera.Pos = vec3(-217, 32, -358);

    DebugCamera.Rot  = vec2(0);
    DebugCamera.Zoom = 1;

    DebugCamera.Sun = createSphere(2, 32, 32);

    DebugLineShader = CreateShaders("../shaders/shape.vert", "../shaders/shape.frag");

    ShadowMap = SetupShadowMapTexture(1080 * 2, 1080 * 2);

    InitImGUI(Window);

    InitPhysics();

    AddContent();

    Bloom = SetupBloom();

    // -----------------------------Water-------------------------------------------

    Water = CreateWater();

    // MainCCT->setPosition(PxExtendedVec3(280, -140, 147));

    glEnable(GL_FRAMEBUFFER_SRGB); 
}

b32 PhysXDebug = false;

// timing
r32 dt        = 0.0f;
r32 LastFrame = 0.0f;

r32 AmbientValue = 0.15f;
r32 SpecValue    = 0.3f;
r32 AlphaEpsi    = 0.f;

vec4 LightColor = vec4(1.f);
vec3 LightPos   = vec3(1, 1, .7) * 1000.f;

extern r32 FOV;

r32 AspectRatio = (r32) WIDTH / (r32) HEIGHT;

r32 FarPlane  = 10000.f;
r32 NearPlane = 0.1f;

r32 PercentageCloserFilter = 0.0009f;

void PrepShader(u32 &ShaderID, mat4 &LightSpaceMatrix, mat4 &Projection, mat4 &View, vec4 &ClipPlane);

mat4 GetReflectionMatrix(camera &Camera);
void RenderScene(u32 FBO, s32 Width, s32 Height,
                 mat4 &LightSpaceMatrix, mat4 &Projection, mat4 &View, mat4 &SkyView, vec4 ClipPlane = vec4(1.f));

void UpdateAndRender(GLFWwindow *Window)
{
    // ------------------------------Timing------------------------------------
    r32 CurrentFrame = (r32) glfwGetTime();
    dt               = CurrentFrame - LastFrame;
    LastFrame        = CurrentFrame;

    StepPhysics(dt);
    // ------------------------------Input-------------------------------------
    if (!ImGuiIo->WantCaptureMouse)
    {
        HandleMouseInputs(&Camera, Window);
    }
    HandleKeyInputs(&Camera, Window, dt);
    UpdateCamera(&Camera);

    // --------------------------Animated Tranformation------------------------
    for (u32 I = 0; I < DynamicModelRenderGroup.Models.size(); ++I)
    {

        DynamicModelRenderGroup.Transforms[I] = mat4(1.0f);
        DynamicModelRenderGroup.Transforms[I] = translate(DynamicModelRenderGroup.Transforms[I], fromPxExtVec3(MainCCT->getFootPosition()));
        DynamicModelRenderGroup.Transforms[I] = rotate(DynamicModelRenderGroup.Transforms[I], radians(180.f), vec3(0.f, 1.f, 0.f));
        DynamicModelRenderGroup.Transforms[I] = rotate(DynamicModelRenderGroup.Transforms[I], radians(Player.RotateX), vec3(0.f, 1.f, 0.f));
        DynamicModelRenderGroup.Transforms[I] = scale(DynamicModelRenderGroup.Transforms[I], vec3(DynamicModelRenderGroup.Scale[I]));

        AnimManager *AnimManager = &DynamicModelRenderGroup.AnimManager[I][Player.AnimState];
        CrossFadeAnimation(&Player, &AnimManager->animator, &AnimManager->animation, dt);
        DynamicModelRenderGroup.AnimTransforms[I] = AnimManager->animator.FinalBoneMatrices;
    }

    // ---------------------------Shadow Mapping--------------------------------------
    mat4 LightSpaceMatrix = CalculateLightSpaceMatrix(&Camera);
    RenderPassShadow(LightSpaceMatrix, ShapeRenderGroup, DynamicModelRenderGroup, ShadowMap);

    //-----------------------------Clip Space----------------------------------------
    mat4 Projection = perspective(FOV, AspectRatio, NearPlane, FarPlane);
    mat4 View       = lookAt(Camera.Position, Camera.LookAt, Camera.Up);
    mat4 SkyView    = mat4(mat3(View));// Removing translation component from transformation matrix

    //---------------------------Water Refraction------------------------------------
    {

        glEnable(GL_CLIP_DISTANCE0);
        vec4 ClipPlane = vec4(0, -1, 0, Water.Height);
        RenderScene(Water.RefractFBO, Water.RefractWidth, Water.RefractHeight,
                    LightSpaceMatrix, Projection, View, SkyView, ClipPlane);
    }
    //---------------------------Water Reflection------------------------------------
    {
        vec4 ClipPlane         = vec4(0, 1, 0, -Water.Height);
        mat4 ReflectionView    = GetReflectionMatrix(Camera);
        mat4 ReflectionSkyView = mat4(mat3(ReflectionView));

        RenderScene(Water.ReflectFBO, Water.ReflectWidth, Water.ReflectHeight,
                    LightSpaceMatrix, Projection, ReflectionView, ReflectionSkyView, ClipPlane);
    }

#if 0
        if (Bloom.Enable)
        {
            glDisable(GL_CLIP_DISTANCE0);
            RenderScene(Bloom.FBO, WIDTH, HEIGHT, LightSpaceMatrix, Projection, View, SkyView);
            RenderWater(Projection, View, Water, Camera.ThirdPCamPos, dt);
        } else
#endif
    //---------------------------Final RenderPass------------------------------------

    {
        if (DebugCamera.Enable) { View = DebugViewMatrix(&DebugCamera); }

        glDisable(GL_CLIP_DISTANCE0);
        RenderScene(0, WIDTH, HEIGHT, LightSpaceMatrix, Projection, View, SkyView);
        RenderWater(Projection, View, Water, Camera.Position, dt);
    }

    // ---------------------------DEBUG RENDER--------------------------------------

    if (Bloom.Enable)
    {
        RenderBloom();
    }

    RenderSun(DebugLineShader, Projection, View, LightPos);

    if (DebugCamera.Enable)
    {
        RenderDebugCamera(DebugLineShader, Projection, View);
    }
    if (PhysXDebug)
    {
        PhysXDebugRender(DebugLineShader, Projection, View);
    }

    // ---------ImGUI--------------

    RenderImGui(dt);
}

// ------------------------------------------------------

void PrepShader(u32 &ShaderID, mat4 &LightSpaceMatrix, mat4 &Projection, mat4 &View, vec4 &ClipPlane)
{
    glUseProgram(ShaderID);

    SetMat4Uniform("projection", Projection, ShaderID);
    SetMat4Uniform("view", View, ShaderID);

    SetVec3Uniform("viewPos", Camera.Position, ShaderID);
    SetVec3Uniform("lightPos", LightPos, ShaderID);
    SetVec4Uniform("lightColor", LightColor, ShaderID);

    SetMat4Uniform("lightSpaceMatrix", LightSpaceMatrix, ShaderID);

    SetFloatUniform("pcf", PercentageCloserFilter, ShaderID);

    SetFloatUniform("ambientVal", AmbientValue, ShaderID);
    SetFloatUniform("specVal", SpecValue, ShaderID);
    SetFloatUniform("alphaEPSI", AlphaEpsi, ShaderID);

    SetVec4Uniform("clip_plane", ClipPlane, ShaderID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ShadowMap.DepthMap);
}

void AddContent()
{
    // Objects
    ShapeRenderGroup.ShaderID       = CreateShaders("../shaders/shadow/shape.vert", "../shaders/shadow/shape.frag");
    ShapeRenderGroup.ShadowShaderID = CreateShaders("../shaders/shadow/shadow_map_depth.vert", "../shaders/shadow/shadow_map_depth.frag");

    glUseProgram(ShapeRenderGroup.ShaderID);
    SetTextUniform("shadowMap", 0, ShapeRenderGroup.ShaderID);
    texture Texture = {"diffuseTexture", 1};
    LoadTexture("../../models/textures/checker/512x512 Texel Density Texture 3.png", &Texture);
    SetTextUniform(Texture.type, Texture.unit, ShapeRenderGroup.ShaderID);

    {
        ShapeRenderGroup.Shapes.push_back(CreateCubeTextured(2, 2, 2));
        ShapeRenderGroup.Colors.emplace_back(0.964, 0.329, 0.274, 1);
        ShapeRenderGroup.Textures.push_back(Texture);
        PxRigidDynamic *Box = PxCreateDynamic(*PhysXSDK,
                                              PxTransform(PxVec3(14)),
                                              PxBoxGeometry(2, 2, 2),
                                              *DefaultMaterialPhysX, 100.0f);

        ShapeRenderGroup.PhysXColliders.push_back(Box);
        ScenePhysX->addActor(*Box);
    }
    {
        ShapeRenderGroup.Shapes.push_back(CreateCubeTextured(1, 20, 40));
        ShapeRenderGroup.Colors.emplace_back(0.964, 0.329, 0.274, 1);
        ShapeRenderGroup.Textures.push_back(Texture);
        PxRigidStatic *Wall = PxCreateStatic(*PhysXSDK,
                                             PxTransform(PxVec3(-20, 0, 0)),
                                             PxBoxGeometry(1, 20, 40),
                                             *DefaultMaterialPhysX);

        ShapeRenderGroup.PhysXColliders.push_back((PxRigidDynamic *const) Wall);
        ScenePhysX->addActor(*Wall);
    }

    // Balls

    {
        r32 Size = 2;
        ShapeRenderGroup.Shapes.push_back(createSphere(Size, 32, 32));
        ShapeRenderGroup.Colors.emplace_back(0.003, 0.237, 0.219, 1);
        ShapeRenderGroup.Textures.push_back(Texture);
        PxRigidDynamic *Ball = PxCreateDynamic(*PhysXSDK,
                                               PxTransform(PxVec3(0)),
                                               PxSphereGeometry(Size),
                                               *DefaultMaterialPhysX, 100.0f);

        Ball->setAngularDamping(.5f);
        Ball->setLinearVelocity(PxVec3(0));

        ShapeRenderGroup.PhysXColliders.push_back(Ball);
        ScenePhysX->addActor(*Ball);
    }
    {
        r32 Size = .5;
        ShapeRenderGroup.Shapes.push_back(createSphere(Size, 32, 32));
        ShapeRenderGroup.Colors.emplace_back(0.013, 0.337, 0.619, 1);
        ShapeRenderGroup.Textures.push_back(Texture);
        PxRigidDynamic *RigidBody = PxCreateDynamic(*PhysXSDK,
                                                    PxTransform(PxVec3(2)),
                                                    PxSphereGeometry(Size),
                                                    *DefaultMaterialPhysX, 100.0f);

        ShapeRenderGroup.PhysXColliders.push_back(RigidBody);
        ScenePhysX->addActor(*RigidBody);
    }
    {
        r32 Size = 1.5;
        ShapeRenderGroup.Shapes.push_back(createSphere(Size, 32, 32));
        ShapeRenderGroup.Colors.emplace_back(0.003, 0.137, 0.619, 1);
        ShapeRenderGroup.Textures.push_back(Texture);
        PxRigidDynamic *RigidBody = PxCreateDynamic(*PhysXSDK,
                                                    PxTransform(PxVec3(4)),
                                                    PxSphereGeometry(Size),
                                                    *DefaultMaterialPhysX, 100.0f);

        ShapeRenderGroup.PhysXColliders.push_back(RigidBody);
        ScenePhysX->addActor(*RigidBody);
    }
    {
        r32 Size = 1;
        ShapeRenderGroup.Shapes.push_back(createSphere(Size, 32, 32));
        ShapeRenderGroup.Colors.emplace_back(0.003, 0.337, 0.619, 1);
        ShapeRenderGroup.Textures.push_back(Texture);
        PxRigidDynamic *RigidBody = PxCreateDynamic(*PhysXSDK,
                                                    PxTransform(PxVec3(6)),
                                                    PxSphereGeometry(Size),
                                                    *DefaultMaterialPhysX, 100.0f);

        ShapeRenderGroup.PhysXColliders.push_back(RigidBody);
        ScenePhysX->addActor(*RigidBody);
    }
    //Balls End

    {
        texture PlaneTexture = {"diffuseTexture", 1};
        LoadTexture("../../models/textures/checker/512x512 Texel Density Texture 1.png", &PlaneTexture);
        SetTextUniform(PlaneTexture.type, PlaneTexture.unit, ShapeRenderGroup.ShaderID);

        ShapeRenderGroup.Shapes.push_back(createPlane(0, 50.f, 2));
        ShapeRenderGroup.Colors.emplace_back(0.172, 0.223, 0.254, 1);
        ShapeRenderGroup.Textures.push_back(PlaneTexture);

        PxRigidStatic *groundPlane = PxCreateStatic(*PhysXSDK, PxTransform(PxVec3(0.f, -10.f, 0.f)),
                                                    PxBoxGeometry(50.f, 0.001, 50.f), *DefaultMaterialPhysX);

        ShapeRenderGroup.PhysXColliders.push_back((PxRigidDynamic *const) groundPlane);

        ScenePhysX->addActor(*groundPlane);
    }

    {
        MainCCT     = CreatePhysXCCT();
        model Model = AssimpLoadModel("../../meGame/Resource/models/lady/binary.fbx");
        DynamicModelRenderGroup.Models.push_back(Model);
        std::vector<mat4> VectorTransforms;
        DynamicModelRenderGroup.AnimTransforms.push_back(VectorTransforms);
        DynamicModelRenderGroup.Transforms.emplace_back(1.f);
        DynamicModelRenderGroup.Scale.emplace_back(1.f);

        DynamicModelRenderGroup.ShaderID       = CreateShaders("../shaders/anim.vert", "../shaders/anim.frag");
        DynamicModelRenderGroup.ShadowShaderID = CreateShaders("../shaders/shadow/anim_shadow_map_depth.vert", "../shaders/shadow/anim_shadow_map_depth.frag");

        std::map<std::string, AnimationState> AnimFiles;
        AnimFiles["../../meGame/Resource/models/lady/idle.fbx"]     = Idle;
        AnimFiles["../../meGame/Resource/models/lady/walk.fbx"]     = Walk;
        AnimFiles["../../meGame/Resource/models/lady/jog.fbx"]      = Jog;
        AnimFiles["../../meGame/Resource/models/lady/run.fbx"]      = Run;
        AnimFiles["../../meGame/Resource/models/lady/jump.fbx"]     = Jump;
        AnimFiles["../../meGame/Resource/models/lady/falling.fbx"]  = Fall;
        AnimFiles["../../meGame/Resource/models/lady/walkBack.fbx"] = WalkBack;

        std::map<AnimationState, AnimManager> AnimManager;

        for (auto const &[key, val] : AnimFiles)
        {
            AnimManager[val] = MakeAnimationManager(&Model, key.c_str(), val);
        }
        DynamicModelRenderGroup.AnimManager.push_back(AnimManager);

        Player.AnimState   = Idle;
        Player.currentAnim = &DynamicModelRenderGroup.AnimManager[DynamicModelRenderGroup.Models.size() - 1][Idle].animation;
        Player.prevAnim    = &DynamicModelRenderGroup.AnimManager[DynamicModelRenderGroup.Models.size() - 1][Idle].animation;

        glUseProgram(DynamicModelRenderGroup.ShaderID);
        SetTextUniform("shadowMap", 0, DynamicModelRenderGroup.ShaderID);
    }
    {

        StaticModelRenderGroup.ShaderID       = CreateShaders("../shaders/default.vert", "../shaders/default.frag");
        StaticModelRenderGroup.ShadowShaderID = CreateShaders("../shaders/shadow/shadow_map_depth.vert", "../shaders/shadow/shadow_map_depth.frag");

        const char *ModelFile = "../../models/terrain/binary.fbx";
        model       Model     = AssimpLoadModel(ModelFile);
        StaticModelRenderGroup.Models.push_back(Model);

        vec3 ModelPos = vec3(0, -200, 0);
        vec3 ModelRot(0);
        r32  ModelScale = 0.50f;

        mat4 TerrainTransform = translate(mat4(1.0f), ModelPos);

        StaticModelRenderGroup.Transforms.push_back(scale(TerrainTransform, vec3(ModelScale)));

        std::string     PhysXMeshFile = (std::string(ModelFile) + std::string(".physx"));
        PxTriangleMesh *CollisionMesh;
        if (0)
        {
            std::vector<PxVec3> Vertices = GetAllVerticesFromModel(&Model);
            std::vector<PxU32>  Indices  = GetAllIndicesFromModel(&Model);

            CollisionMesh = createBV34TriangleMesh(PhysXMeshFile.c_str(),
                                                   Vertices.size(), Vertices.data(),
                                                   Indices.size() / 3, Indices.data(),
                                                   false, false, false, 4);
        } else
        {
            CollisionMesh = LoadTriangleMesh(PhysXMeshFile.c_str());
        }

        u32 DebugVAO          = SetUpPhysXMeshForRendering(CollisionMesh);
        u32 DebugIndicesCount = CollisionMesh->getNbTriangles() * 3;
        StaticModelRenderGroup.DebugVAO.push_back(DebugVAO);
        StaticModelRenderGroup.DebugIndicesCount.push_back(DebugIndicesCount);

        PhysXTerrainTransform   = PxTransform(PxMat44(value_ptr(TerrainTransform)));
        PhysXTriangleGeom       = PxTriangleMeshGeometry(CollisionMesh);
        PhysXTriangleGeom.scale = PxMeshScale(ModelScale);

        PxRigidStatic *TerrainStatic = PxCreateStatic(
        *PhysXSDK, PhysXTerrainTransform, PhysXTriangleGeom, *DefaultMaterialPhysX);

        PxShape *Shapes;
        TerrainStatic->getShapes(&Shapes, 1);
        Shapes->setFlag(PxShapeFlag::eVISUALIZATION, false);

        ScenePhysX->addActor(*TerrainStatic);
    }
    // Objects
}

void RenderScene(u32 FBO, s32 Width, s32 Height,
                 mat4 &LightSpaceMatrix, mat4 &Projection, mat4 &View, mat4 &SkyView, vec4 ClipPlane)
{
    glViewport(0, 0, Width, Height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    {
        PrepShader(ShapeRenderGroup.ShaderID, LightSpaceMatrix, Projection, View, ClipPlane);
        RenderShapes(&ShapeRenderGroup, ShapeRenderGroup.ShaderID);
    }

    {
        PrepShader(DynamicModelRenderGroup.ShaderID, LightSpaceMatrix, Projection, View, ClipPlane);
        RenderAnimatedModels(&DynamicModelRenderGroup, DynamicModelRenderGroup.ShaderID);
    }

    {
        PrepShader(StaticModelRenderGroup.ShaderID, LightSpaceMatrix, Projection, View, ClipPlane);
        RenderStaticModels(&StaticModelRenderGroup);
    }
    RenderSkybox(Skybox, SkyView, ClipPlane);
}

mat4 GetReflectionMatrix(camera &Camera)
{
    r32  CamWaterDist  = 2 * (Camera.Position.y - Water.Height);
    vec3 ReflactCamPos = Camera.Position - vec3(0, CamWaterDist, 0);

    vec3 Front = vec3(Camera.Front.x, -Camera.Front.y, Camera.Front.z);

    mat4 ReflectionView = lookAt(ReflactCamPos, ReflactCamPos + Front, Camera.Up);

    return ReflectionView;
}
