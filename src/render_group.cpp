//
// Created by AgentOfChaos on 4/8/2022.
//

#include "render_group.h"
#include "shader.h"

extern b32     PhysXDebug;
extern PxScene *ScenePhysX;
extern ImGuiIO *ImGuiIo;

void InitImGUI(GLFWwindow *Window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(Window, true);
    ImGui_ImplOpenGL3_Init("#version 430 core");

    ImGuiIo = &io;
}

extern r32          PercentageCloserFilter;
extern debug_camera DebugCamera;

void RenderImGui(r32 dt)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    std::string DeltaText = std::to_string(dt * 1000.0f);
    std::string FPSText   = std::to_string(1.f / dt);
    ImGui::Begin("Timing");
    ImGui::Text("%s", DeltaText.c_str());
    ImGui::Text("%s", FPSText.c_str());
    ImGui::End();

    ImGui::Begin("Shadows");
    ImGui::InputFloat("PCF", &PercentageCloserFilter);
    ImGui::End();

    ImGui::Begin("DebugCamera");
    ImGui::Checkbox("Enable", &DebugCamera.Enable);
    ImGui::Checkbox("ShadowsBox", &DebugCamera.EnableShadowBox);
    ImGui::Checkbox("ShadowsBox", &DebugCamera.EnableShadowBox);

    ImGui::InputFloat3("Position", &DebugCamera.Pos.x);
    ImGui::SliderFloat2("Rotation", &DebugCamera.Rot.x, -1.f, 1.f);
    ImGui::SliderFloat("Zoom", &DebugCamera.Zoom, 0.9f, 1.1f);

    ImGui::Checkbox("PhysXDebug", &PhysXDebug);

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

extern r32  FOV;
extern r32  AspectRatio;
extern vec3 LightPos;
extern r32  ShadowFarDist;

r32 FOV_Radians    = radians(FOV);
r32 ShadowNearDist = 0.01f;

r32 Hnear = 2 * tan(FOV_Radians / 2) * ShadowNearDist;
r32 Wnear = Hnear * AspectRatio;
r32 Hfar  = 2 * tan(FOV_Radians / 2) * ShadowFarDist;
r32 Wfar  = Hfar * AspectRatio;

vec3 ShadowUP(0, 1, 0);
vec2 Zs(0);

mat4 CalculateLightSpaceMatrix(camera *Camera)
{
    // Shadow Map render PASS
    mat4 LightView        = lookAt(normalize(LightPos), vec3(0, 0, 0), ShadowUP);
    mat4 ShadowProjection = perspective(FOV_Radians, AspectRatio, ShadowNearDist, ShadowFarDist);

    mat4 inverseProjectViewMatrix = inverse(ShadowProjection * GetViewMatrix(Camera));

    // clang-format off
    std::vector<glm::vec4> NDC =
                           {
                           vec4{-1.0f, -1.0f, 1.0f, 1.0f},
                           vec4{ 1.0f, -1.0f, 1.0f, 1.0f},
                           vec4{ 1.0f, -1.0f, 0.0f, 1.0f},
                           vec4{-1.0f, -1.0f, 0.0f, 1.0f},
                           vec4{-1.0f,  1.0f, 1.0f, 1.0f},
                           vec4{ 1.0f,  1.0f, 1.0f, 1.0f},
                           vec4{ 1.0f,  1.0f, 0.0f, 1.0f},
                           vec4{-1.0f,  1.0f, 0.0f, 1.0f},

                           };

    std::vector<vec4> DebugFrustumView =
                      {
                      vec4{-1.0f, -1.0f, 1.0f, 1.0f},
                      vec4{ 1.0f, -1.0f, 1.0f, 1.0f},
                      vec4{ 1.0f, -1.0f, 0.0f, 1.0f},
                      vec4{-1.0f, -1.0f, 0.0f, 1.0f},
                      vec4{-1.0f,  1.0f, 1.0f, 1.0f},
                      vec4{ 1.0f,  1.0f, 1.0f, 1.0f},
                      vec4{ 1.0f,  1.0f, 0.0f, 1.0f},
                      vec4{-1.0f,  1.0f, 0.0f, 1.0f},
                      };
    // clang-format on

    for (size_t i = 0; i < NDC.size(); i++)
    {
        NDC[i] = LightView * inverseProjectViewMatrix * NDC[i];
        NDC[i] /= NDC[i].w;

        DebugFrustumView[i] = inverseProjectViewMatrix * DebugFrustumView[i];
        DebugFrustumView[i] /= DebugFrustumView[i].w;
    }

    vec3              min = {INFINITY, INFINITY, INFINITY};
    vec3              max = {-INFINITY, -INFINITY, -INFINITY};
    for (unsigned int i   = 0; i < NDC.size(); i++)
    {
        if (NDC[i].x < min.x)
            min.x = NDC[i].x;
        if (NDC[i].y < min.y)
            min.y = NDC[i].y;
        if (NDC[i].z < min.z)
            min.z = NDC[i].z;

        if (NDC[i].x > max.x)
            max.x = NDC[i].x;
        if (NDC[i].y > max.y)
            max.y = NDC[i].y;
        if (NDC[i].z > max.z)
            max.z = NDC[i].z;
    }

    mat4 LightProjection = ortho(min.x, max.x, min.y, max.y, -max.z, -min.z);

    mat4 LightSpaceMatrix = LightProjection * LightView;

    //-----------------------------------------;
    DebugCamera.LightView = LightView;
    DebugCamera.Frustum   = DebugFrustumView;
    DebugCamera.Min       = min;
    DebugCamera.Max       = max;
    //-----------------------------------------;

    return LightSpaceMatrix;
}

void RenderShapes(render_group_shape *Group, u32 ShaderID)
{
    for (u32 I = 0; I < Group->Shapes.size(); ++I)
    {

        PxMat44 ModelMatrixPX(Group->PhysXColliders[I]->getGlobalPose());
        mat4    ModelMatrix = glm::make_mat4(&ModelMatrixPX.column0.x);

        SetMat4Uniform("model", ModelMatrix, ShaderID);
        SetVec4Uniform("Color", Group->Colors[I], ShaderID);

        glActiveTexture(GL_TEXTURE0 + Group->Textures[I].unit);
        glBindTexture(GL_TEXTURE_2D, Group->Textures[I].ID);

        RenderShape(&Group->Shapes[I]);
    }
}

void RenderAnimatedModels(dynamic_model_render_group *Group, u32 ShaderID)
{

    for (u32 I = 0; I < Group->Models.size(); ++I)
    {
        SetMat4Uniform("model", Group->Transforms[I], ShaderID);
        for (int J = 0; J < Group->AnimTransforms[I].size(); ++J)
        {
            std::string Name = "finalBonesTransformations[" + std::to_string(J) + "]";
            SetMat4Uniform(Name.c_str(), Group->AnimTransforms[I][J], ShaderID);
        }

        DrawModel(&Group->Models[I], ShaderID);
    }
}

shadow_map SetupShadowMapTexture(s32 Width, s32 Height)
{
    shadow_map Result = {Width, Height};

    // Shadow map texture
    glGenFramebuffers(1, &Result.FBO);
    // create depth texture
    glGenTextures(1, &Result.DepthMap);
    glBindTexture(GL_TEXTURE_2D, Result.DepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, Result.FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Result.DepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Shadow map texture END
    return Result;
}

void RenderPassShadow(mat4 &LightSpaceMatrix, render_group_shape &GroupShape, dynamic_model_render_group &DynamicModelRenderGroup, shadow_map &ShadowMap)
{
    glViewport(0, 0, ShadowMap.Width, ShadowMap.Height);
    glBindFramebuffer(GL_FRAMEBUFFER, ShadowMap.FBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(GroupShape.ShadowShaderID);
    SetMat4Uniform("lightSpaceMatrix", LightSpaceMatrix, GroupShape.ShadowShaderID);
    RenderShapes(&GroupShape, GroupShape.ShadowShaderID);

    glUseProgram(DynamicModelRenderGroup.ShadowShaderID);
    SetMat4Uniform("lightSpaceMatrix", LightSpaceMatrix, DynamicModelRenderGroup.ShadowShaderID);

    RenderAnimatedModels(&DynamicModelRenderGroup, DynamicModelRenderGroup.ShadowShaderID);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WIDTH, HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

mat4 DebugViewMatrix(debug_camera *Camera)
{
    mat4 View;
    quat qx = glm::angleAxis(radians(Camera->Rot.x), vec3(0.f, 1.f, 0.f));
    quat qy = glm::angleAxis(radians(Camera->Rot.y), vec3(0.f, 0.f, 1.f));

    Camera->Pos = Camera->Pos * qx;
    Camera->Pos = Camera->Pos * qy;

    Camera->Pos  = vec3(glm::scale(mat4(1.f), vec3(Camera->Zoom)) * vec4(Camera->Pos, 1.f));
    Camera->Zoom = 1;
    Camera->Rot  = vec2(0);
    View = lookAt(Camera->Pos, vec3(0), vec3(0, 1, 0));
    return View;
}

void RenderDebugCamera(u32 &ShaderID, mat4 &Projection, mat4 &View)
{// LIGHT POS DEBUG == SUN
// SetMat4Uniform("model", mat4(1), DebugLineShader);
// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glUseProgram(ShaderID);
    SetMat4Uniform("projection", Projection, ShaderID);
    SetMat4Uniform("view", View, ShaderID);

    SetMat4Uniform("model", DebugCamera.LightView, ShaderID);

    // Shape BoxOBB = createBoxAA(min.x, max.x, min.y, max.y, -max.z, -min.z);
    shape BoxOBB = CreateBoxMinMax(DebugCamera.Min.x, DebugCamera.Max.x,
                                   DebugCamera.Min.y, DebugCamera.Max.y,
                                   DebugCamera.Min.z, DebugCamera.Max.z);

    glDisable(GL_CULL_FACE);

    if (DebugCamera.EnableShadowBox)
    {
        // Right
        SetVec4Uniform("Color", vec4(1, 1, 0, 0.5), ShaderID);
        RenderSide(&BoxOBB, 0);
        // Left
        SetVec4Uniform("Color", vec4(0, 1, 0, 0.5), ShaderID);
        RenderSide(&BoxOBB, 1);
        // Top
        SetVec4Uniform("Color", vec4(1, 0, 0, 0.5), ShaderID);
        RenderSide(&BoxOBB, 2);
        // Bottom
        SetVec4Uniform("Color", vec4(0, 0, 1, 0.5), ShaderID);
        RenderSide(&BoxOBB, 3);
        // Front
        SetVec4Uniform("Color", vec4(.5, .5, .5, .5), ShaderID);
        RenderSide(&BoxOBB, 4);
        // Back
        SetVec4Uniform("Color", vec4(0, 1, 1, 0.5), ShaderID);
        RenderSide(&BoxOBB, 5);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    SetMat4Uniform("model", mat4(1), ShaderID);
    SetVec4Uniform("Color", vec4(1, 0, 1, 1), ShaderID);
    shape ViewFrustum = CreateFrustum(DebugCamera.Frustum);
    RenderShape(&ViewFrustum);
    glEnable(GL_CULL_FACE);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void RenderSun(u32 &ShaderID, mat4 &Projection, mat4 &View, vec3 &Pos)
{
    glUseProgram(ShaderID);
    mat4 LightModel = glm::translate(mat4(1.f), Pos);
    SetMat4Uniform("projection", Projection, ShaderID);
    SetMat4Uniform("view", View, ShaderID);

    SetMat4Uniform("model", LightModel, ShaderID);
    SetVec4Uniform("Color", vec4(1, 1, 1, 1), ShaderID);
    RenderShape(&DebugCamera.Sun);
}

void PhysXDebugRender(u32 &ShaderID, mat4 &Projection, mat4 &View)
{
    glUseProgram(ShaderID);
    SetMat4Uniform("projection", Projection, ShaderID);
    SetMat4Uniform("view", View, ShaderID);
    SetMat4Uniform("model", mat4(1.f), ShaderID);
    SetVec4Uniform("Color", vec4(0, 1, 0, 1), ShaderID);

    const PxRenderBuffer &Buffer = ScenePhysX->getRenderBuffer();
    std::vector<PxVec3>  Vertices;

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
