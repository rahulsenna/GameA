//
// Created by AgentOfChaos on 4/5/2022.
//

#include "game.h"

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

shape CreateQuad();

shape Quad;

void AddContent();

bloom_properties Bloom;

void InitGame(GLFWwindow *Window)
{
    Camera = InitCamera(glm::vec3(0, 2, 4));

    DebugCamera.Enable          = false;
    DebugCamera.EnableShadowBox = true;
    DebugCamera.Pos             = vec3(500, 200, -100);
    DebugCamera.Rot             = vec2(0);
    DebugCamera.Zoom            = 1;

    DebugCamera.Sun = createSphere(2, 32, 32);

    DebugLineShader = CreateShaders("../shaders/shape.vert", "../shaders/shape.frag");

    ShadowMap = SetupShadowMapTexture(1080 * 2, 1080 * 2);

    InitImGUI(Window);

    InitPhysics();

    AddContent();

    Quad = CreateQuad();
    // Bloom

    Bloom = SetupBloom();
}

b32 PhysXDebug = true;

// timing
r32 dt        = 0.0f;
r32 LastFrame = 0.0f;

r32 AmbientValue = 0.5f;
r32 SpecValue    = 0.3f;
r32 AlphaEpsi    = 0.f;
r32 ScalePlayer  = 1;

vec4 LightColor = vec4(1.f);
vec3 LightPos   = vec3(0.5f, 2, 2) * 30.f;

extern r32 FOV;

r32 AspectRatio = (r32) WIDTH / (r32) HEIGHT;

r32 farPlane               = 10000.f;
r32 PercentageCloserFilter = 0.0009f;
r32 ShadowFarDist          = 200;

void PrepShader(u32 &ShaderID, mat4 &LightSpaceMatrix, mat4 &Projection, mat4 &View);

void UpdateAndRender(GLFWwindow *Window)
{
    r32 CurrentFrame = glfwGetTime();
    dt               = CurrentFrame - LastFrame;
    LastFrame        = CurrentFrame;

    StepPhysics(dt);

    if (!ImGuiIo->WantCaptureMouse)
    {
        HandleMouseInputs(&Camera, Window);
    }
    HandleKeyInputs(&Camera, Window, dt);
    UpdateCamera(&Camera);

    for (u32 I = 0; I < DynamicModelRenderGroup.Models.size(); ++I)
    {

        DynamicModelRenderGroup.Transforms[I] = mat4(1.0f);
        DynamicModelRenderGroup.Transforms[I] = translate(DynamicModelRenderGroup.Transforms[I], V3PxVec3(MainCCT->getFootPosition()));
        DynamicModelRenderGroup.Transforms[I] = rotate(DynamicModelRenderGroup.Transforms[I], radians(180.f), vec3(0.f, 1.f, 0.f));
        DynamicModelRenderGroup.Transforms[I] = rotate(DynamicModelRenderGroup.Transforms[I], radians(Player.RotateX), vec3(0.f, 1.f, 0.f));
        DynamicModelRenderGroup.Transforms[I] = scale(DynamicModelRenderGroup.Transforms[I], vec3(ScalePlayer));

        AnimManager *AnimManager = &DynamicModelRenderGroup.AnimManager[I][Player.AnimState];
        CrossFadeAnimation(&Player, &AnimManager->animator, &AnimManager->animation, dt);
        DynamicModelRenderGroup.AnimTransforms[I] = AnimManager->animator.FinalBoneMatrices;
    }

    mat4 LightSpaceMatrix = CalculateLightSpaceMatrix(&Camera);

    RenderPassShadow(LightSpaceMatrix, ShapeRenderGroup, DynamicModelRenderGroup, ShadowMap);

    //------------------------------------------------
    if (Bloom.Enable)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, Bloom.FBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    mat4 Projection = perspective(FOV, AspectRatio, 0.01f, 10000.f);
    mat4 View       = GetViewMatrix(&Camera);

    if (DebugCamera.Enable)
    {
        View = DebugViewMatrix(&DebugCamera);
    }

    {
        PrepShader(ShapeRenderGroup.ShaderID, LightSpaceMatrix, Projection, View);
        RenderShapes(&ShapeRenderGroup, ShapeRenderGroup.ShaderID);
    }

    {
        PrepShader(DynamicModelRenderGroup.ShaderID, LightSpaceMatrix, Projection, View);
        RenderAnimatedModels(&DynamicModelRenderGroup, DynamicModelRenderGroup.ShaderID);
    }

    {
        PrepShader(StaticModelRenderGroup.ShaderID, LightSpaceMatrix, Projection, View);

        for (u32 I = 0; I < StaticModelRenderGroup.Models.size(); ++I)
        {
            SetMat4Uniform("model", StaticModelRenderGroup.Transforms[I], StaticModelRenderGroup.ShaderID);
            DrawModel(&StaticModelRenderGroup.Models[I], StaticModelRenderGroup.ShaderID);
            if (0)
            {
                RenderPhysXMesh(StaticModelRenderGroup.DebugVAO[I], DebugLineShader,
                                StaticModelRenderGroup.DebugIndicesCount[I],
                                View, Projection, StaticModelRenderGroup.Transforms[I]);
            }
        }
    }

    // -----------------------------------------------------------------
    //                   DEBUG RENDER
    RenderSun(DebugLineShader, Projection, View, LightPos);

    if (DebugCamera.Enable)
    {
        RenderDebugCamera(DebugLineShader, Projection, View);
    }
    if (PhysXDebug)
    {
        PhysXDebugRender(DebugLineShader, Projection, View);
    }

    if (Bloom.Enable)
    {
        RenderBloom(&Quad);
    }

    // -----------------------
    RenderImGui(dt);
}

// ------------------------------------------------------

void PrepShader(u32 &ShaderID, mat4 &LightSpaceMatrix, mat4 &Projection, mat4 &View)
{
    glUseProgram(ShaderID);

    SetMat4Uniform("projection", Projection, ShaderID);
    SetMat4Uniform("view", View, ShaderID);

    SetVec3Uniform("viewPos", Camera.ThirdPCamPos, ShaderID);
    SetVec3Uniform("lightPos", LightPos, ShaderID);
    SetVec4Uniform("lightColor", LightColor, ShaderID);

    SetMat4Uniform("lightSpaceMatrix", LightSpaceMatrix, ShaderID);

    SetFloatUniform("pcf", PercentageCloserFilter, ShaderID);

    SetFloatUniform("ambientVal", AmbientValue, ShaderID);
    SetFloatUniform("specVal", SpecValue, ShaderID);
    SetFloatUniform("alphaEPSI", AlphaEpsi, ShaderID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ShadowMap.DepthMap);
}

void AddContent()
{
    // Objects
    ShapeRenderGroup.ShaderID       = CreateShaders("../shadow/3.1.3.shadow_mapping.vs", "../shadow/3.1.3.shadow_mapping.fs");
    ShapeRenderGroup.ShadowShaderID = CreateShaders("../shadow/3.1.3.shadow_mapping_depth.vs", "../shadow/3.1.3.shadow_mapping_depth.fs");

    glUseProgram(ShapeRenderGroup.ShaderID);
    SetTextUniform("shadowMap", 0, ShapeRenderGroup.ShaderID);
    texture Texture = {"diffuseTexture", 1};
    LoadTexture("C:/Users/AgentOfChaos/Desktop/models/textures/checker/512x512 Texel Density Texture 3.png", &Texture);
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
        LoadTexture("C:/Users/AgentOfChaos/Desktop/models/textures/checker/512x512 Texel Density Texture 1.png", &PlaneTexture);
        SetTextUniform(PlaneTexture.type, PlaneTexture.unit, ShapeRenderGroup.ShaderID);

        ShapeRenderGroup.Shapes.push_back(createPlane(0, 50.f, 2));
        ShapeRenderGroup.Colors.emplace_back(0.172, 0.223, 0.254, 1);
        ShapeRenderGroup.Textures.push_back(PlaneTexture);

        PxRigidStatic *groundPlane = PxCreateStatic(*PhysXSDK, PxTransform(PxVec3(0.f, -10.f, 0.f)),
                                                    PxBoxGeometry(50.f, 0.001, 50.f), *DefaultMaterialPhysX);

        ShapeRenderGroup.PhysXColliders.push_back((PxRigidDynamic *const) groundPlane);

        ScenePhysX->addActor(*groundPlane);
    }
    // 0.933, 0.949, 0.960 lightGray
    //    0.172, 0.223, 0.254  gray

    {
        MainCCT     = CreatePhysXCCT();
        model Model = AssimpLoadModel("C:/Users/AgentOfChaos/Desktop/meGame/Resource/models/lady/binary.fbx");
        DynamicModelRenderGroup.Models.push_back(Model);
        std::vector<mat4> VectorTransforms;
        DynamicModelRenderGroup.AnimTransforms.push_back(VectorTransforms);
        DynamicModelRenderGroup.Transforms.emplace_back(1.f);

        DynamicModelRenderGroup.ShaderID       = CreateShaders("../shaders/anim.vert", "../shaders/anim.frag");
        DynamicModelRenderGroup.ShadowShaderID = CreateShaders("../shadow/anim_shadow_depth.vs", "../shadow/anim_shadow_depth.fs");

        std::map<std::string, AnimationState> AnimFiles;
        AnimFiles["C:/Users/AgentOfChaos/Desktop/meGame/Resource/models/lady/idle.fbx"]     = Idle;
        AnimFiles["C:/Users/AgentOfChaos/Desktop/meGame/Resource/models/lady/walk.fbx"]     = Walk;
        AnimFiles["C:/Users/AgentOfChaos/Desktop/meGame/Resource/models/lady/jog.fbx"]      = Jog;
        AnimFiles["C:/Users/AgentOfChaos/Desktop/meGame/Resource/models/lady/run.fbx"]      = Run;
        AnimFiles["C:/Users/AgentOfChaos/Desktop/meGame/Resource/models/lady/jump.fbx"]     = Jump;
        AnimFiles["C:/Users/AgentOfChaos/Desktop/meGame/Resource/models/lady/falling.fbx"]  = Fall;
        AnimFiles["C:/Users/AgentOfChaos/Desktop/meGame/Resource/models/lady/walkBack.fbx"] = WalkBack;

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
        StaticModelRenderGroup.ShadowShaderID = CreateShaders("../shadow/3.1.3.shadow_mapping_depth.vs", "../shadow/3.1.3.shadow_mapping_depth.fs");

        const char *ModelFile = "C:/Users/AgentOfChaos/Desktop/models/terrain/binary.fbx";
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

        PxMat44                PhysXTerrainTransform = PxMat44(value_ptr(TerrainTransform));
        PxTriangleMeshGeometry TriangleGeom          = PxTriangleMeshGeometry(CollisionMesh);
        TriangleGeom.scale                           = PxMeshScale(ModelScale);

        PxRigidStatic *TerrainStatic = PxCreateStatic(
        *PhysXSDK, PxTransform(PhysXTerrainTransform), TriangleGeom, *DefaultMaterialPhysX);

        PxShape *Shapes;
        TerrainStatic->getShapes(&Shapes, 1);
        Shapes->setFlag(PxShapeFlag::eVISUALIZATION, false);

        ScenePhysX->addActor(*TerrainStatic);
    }
    // Objects
}
