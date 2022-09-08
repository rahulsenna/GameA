//
// Created by AgentOfChaos on 4/8/2022.
//

#include "render_group.h"
#include "shader.h"

extern b32      PhysXDebug;
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

extern r32              PercentageCloserFilter;
extern debug_camera     DebugCamera;
extern bloom_properties Bloom;

extern PxController *MainCCT;

extern shadow_map ShadowMap;

extern water Water;

extern vec3 LightPos;
extern vec4 LightColor;
vec3        LightRotate(0);

void RenderImGui(r32 dt)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    std::string DeltaText = std::to_string(dt * 1000.0f);
    std::string FPSText   = std::to_string(1.f / dt);
    ImGui::Begin("Timing");
    ImGui::Text("%s ms", DeltaText.c_str());
    ImGui::Text("%s FPS", FPSText.c_str());
    ImGui::End();

    ImGui::Begin("Shadows");
    ImGui::InputFloat("PCF", &PercentageCloserFilter);
    ImGui::End();

    ImGui::Begin("DebugCamera");
    ImGui::Checkbox("Enable", &DebugCamera.Enable);
    ImGui::Checkbox("ShadowsBox", &DebugCamera.EnableShadowBox);

    ImGui::InputFloat3("Position", &DebugCamera.Pos.x);
    ImGui::SliderFloat2("Rotation", &DebugCamera.Rot.x, -1.f, 1.f);
    ImGui::SliderFloat("Zoom", &DebugCamera.Zoom, 0.9f, 1.1f);
    ImGui::Checkbox("PhysXDebug", &PhysXDebug);
    ImGui::End();

    ImGui::Begin("Bloom");
    ImGui::Checkbox("Enable", &Bloom.Enable);
    ImGui::InputFloat("Exposure", &Bloom.exposure);
    ImGui::End();

    ImGui::Begin("Light");
    ImGui::InputFloat3("Pos", &LightPos.x);
    ImGui::ColorEdit4("Color", &LightColor.x);
    ImGui::SliderFloat3("Rotation", &LightRotate.x, -1.f, 1.f);
    ImGui::End();

    ImGui::Begin("WaterMatrix");

    // ImGui::SliderFloat("Depth", &WaterTranslate.y, -400, 1);

    // ImGui::SliderFloat3("Position", &WaterTranslate.x, -550, 1000);
    // ImGui::InputFloat3("Rotate", &WaterRotate.x);
    // ImGui::SliderFloat3("Scale", &WaterScale.x, 100, 500);

    glm::quat qx = glm::angleAxis(radians(LightRotate.x), glm::vec3(0.f, 1.f, 0.f));
    glm::quat qy = glm::angleAxis(-radians(LightRotate.y), glm::vec3(1.f, 0.f, 0.f));
    glm::quat qz = glm::angleAxis(-radians(LightRotate.z), glm::vec3(0.f, 0.f, 1.f));

    LightPos = LightPos * qx;
    LightPos = LightPos * qy;
    LightPos = LightPos * qz;

    LightRotate = vec3(0);

    ImGui::End();

    // ImGui::Begin("WaterReflection");
    // ImGui::Image((void *) (intptr_t) Water.Reflection.ID, ImVec2(704, 396));
    // ImGui::End();
    // ImGui::Begin("WaterRefraction");
    // ImGui::Image((void *) (intptr_t) Water.Refraction.ID, ImVec2(704, 396));
    // ImGui::End();

    // ImGui::Begin("WaterDepth");
    // ImGui::Image((void *) (intptr_t) WaterDepthMap.ID, ImVec2(704, 396));
    // ImGui::Image((void *) (intptr_t) ShadowMap.DepthMap, ImVec2(704, 396));

    // ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

extern r32 FOV;
extern r32 AspectRatio;

mat4 GetReflectionMatrix(camera &Camera);

mat4 CalculateLightSpaceMatrix(camera *Camera)
{
    // Shadow Map render PASS
    mat4 LightView        = lookAt(normalize(LightPos), vec3(0, 0, 0), ShadowMap.Up);
    mat4 ShadowProjection = perspective(FOV, AspectRatio, ShadowMap.NearPlane, ShadowMap.FarPlane);

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

    vec3 min = {INFINITY, INFINITY, INFINITY};
    vec3 max = {-INFINITY, -INFINITY, -INFINITY};
    for (unsigned int i = 0; i < NDC.size(); i++)
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

        BindTexture(Group->Textures[I]);
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
void RenderStaticModels(static_model_render_group *Group)
{
    for (u32 I = 0; I < Group->Models.size(); ++I)
    {
        SetMat4Uniform("model", Group->Transforms[I], Group->ShaderID);
        DrawModel(&Group->Models[I], Group->ShaderID);
#if 0
            if (DrawDebugTriangleMesh)
                    {
                        RenderPhysXMesh(Group->DebugVAO[I], DebugLineShader,
                                        Group->DebugIndicesCount[I],
                                        View, Projection, Group->Transforms[I]);
                    }
#endif
    }
}

shadow_map SetupShadowMapTexture(s32 Width, s32 Height)
{
    shadow_map Result = {Width, Height};

    Result.FarPlane  = 200;
    Result.NearPlane = 0.01f;
    Result.Up        = vec3(0, 1, 0);

    Result.FBO      = CreateFBO();
    Result.DepthMap = DepthTextureAttachment(Width, Height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    r32 BorderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BorderColor);
    // attach depth texture as FBO's depth buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Shadow map texture END
    return Result;
}

void RenderPassShadow(mat4                       &LightSpaceMatrix,
                      render_group_shape         &GroupShape,
                      dynamic_model_render_group &DynamicModelRenderGroup,
                      shadow_map                 &ShadowMap)
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
    View         = lookAt(Camera->Pos, vec3(0), vec3(0, 1, 0));
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

bloom_properties SetupBloom()
{
    bloom_properties Bloom;
    Bloom.BloomShader = CreateShaders("../shaders/post_processing/bloom.vert", "../shaders/post_processing/bloom.frag");
    Bloom.BlurShader  = CreateShaders("../shaders/post_processing/blur.vert", "../shaders/post_processing/blur.frag");

    // configure (floating point) framebuffers
    // ---------------------------------------
    Bloom.FBO = CreateFBO();

    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    glGenTextures(2, Bloom.colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, Bloom.colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, Bloom.colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)

    Bloom.rboDepth = DepthBufferAttachment(WIDTH, HEIGHT);

    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    u32 attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    // finally check if framebuffer is complete;
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "BloomFBO not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring

    glGenFramebuffers(2, Bloom.pingpongFBO);
    glGenTextures(2, Bloom.pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, Bloom.pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, Bloom.pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Bloom.pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    glUseProgram(Bloom.BlurShader);
    SetTextUniform("image", 0, Bloom.BlurShader);

    glUseProgram(Bloom.BloomShader);
    SetTextUniform("scene", 0, Bloom.BloomShader);
    SetTextUniform("bloomBlur", 1, Bloom.BloomShader);

    Bloom.Quad = CreateQuad();

    return Bloom;
}

void RenderBloom()
{
#if 1
    // 2. blur bright fragments with two-pass Gaussian Blur
    // --------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bool         horizontal = true, first_iteration = true;
    unsigned int amount = 10;
    glUseProgram(Bloom.BlurShader);

    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, Bloom.pingpongFBO[horizontal]);
        SetIntUniform("horizontal", horizontal, Bloom.BlurShader);
        glBindTexture(GL_TEXTURE_2D,
                      first_iteration ? Bloom.colorBuffers[1] : Bloom.pingpongColorbuffers[!horizontal]);
        // bind texture of other framebuffer (or scene if first iteration)
        RenderShape(&Bloom.Quad);

        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //
    // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer'
    // s (clamped) color range
    // ----------------------------------------------------------------------------------------------------
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(Bloom.BloomShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Bloom.colorBuffers[0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, Bloom.pingpongColorbuffers[!horizontal]);
    // glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[0]);

    SetIntUniform("bloom", Bloom.Enable, Bloom.BloomShader);
    SetFloatUniform("exposure", Bloom.exposure, Bloom.BloomShader);

    RenderShape(&Bloom.Quad);
}

void RenderSkybox(cubemap &Cubemap, mat4 &view, vec4 &ClipPlane)
{
    glDepthFunc(GL_LEQUAL);
    glUseProgram(Cubemap.ShaderID);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (r32) WIDTH / HEIGHT, 0.1f, 100.0f);
    SetMat4Uniform("view", view, Cubemap.ShaderID);
    SetMat4Uniform("projection", projection, Cubemap.ShaderID);

    SetVec4Uniform("clip_plane", ClipPlane, Cubemap.ShaderID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Cubemap.Texture);
    RenderShape(&Cubemap.Cube);

    glDepthFunc(GL_LESS);
}

cubemap CreateCubemap()
{
    cubemap Cubemap  = {};
    Cubemap.ShaderID = CreateShaders("../shaders/skybox.vert", "../shaders/skybox.frag");
    Cubemap.Cube     = createBox(1, 1, 1);

    glUseProgram(Cubemap.ShaderID);
    glUniform1i(glGetUniformLocation(Cubemap.ShaderID, "skybox"), 0);
    glGenTextures(1, &Cubemap.Texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Cubemap.Texture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    std::string images[6] =
    {
    "../../models/skybox3/right.jpg",
    "../../models/skybox3/left.jpg",
    "../../models/skybox3/top.jpg",
    "../../models/skybox3/bottom.jpg",
    "../../models/skybox3/front.jpg",
    "../../models/skybox3/back.jpg",
    };

    stbi_set_flip_vertically_on_load(false);
    for (u8 i = 0; i < 6; ++i)
    {
        int            imgWidth, imgHeight, numColCh;
        unsigned char *bytes = stbi_load(images[i].c_str(), &imgWidth, &imgHeight, &numColCh, 0);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     GL_RGB,
                     imgWidth, imgHeight,
                     0,
                     GL_RGB,
                     GL_UNSIGNED_BYTE,
                     bytes);
        stbi_image_free(bytes);
    }

    return Cubemap;
}

void RenderWater(mat4 &Projection, mat4 &View, water &Water, vec3 &CamPos, r32 &dt)
{
    glUseProgram(Water.Shader);
    SetMat4Uniform("Projection", Projection, Water.Shader);
    SetMat4Uniform("View", View, Water.Shader);
    SetMat4Uniform("Model", Water.Tranformation, Water.Shader);

    Water.MoveFactor += 0.05f * dt;
    if (Water.MoveFactor > 1.f) { Water.MoveFactor = 0; }

    SetFloatUniform("MoveFactor", Water.MoveFactor, Water.Shader);
    SetVec3Uniform("CameraPos", CamPos, Water.Shader);

    SetVec3Uniform("LightPos", LightPos, Water.Shader);
    SetVec4Uniform("LightColor", LightColor, Water.Shader);

    BindTexture(Water.Reflection);
    BindTexture(Water.Refraction);
    BindTexture(Water.DUDV);
    BindTexture(Water.Normal);
    BindTexture(Water.DepthMap);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    RenderShape(&Water.Quad);
    glDisable(GL_BLEND);
}

water CreateWater()
{
    water Water      = {};
    Water.Shader     = CreateShaders("../shaders/water/water.vert", "../shaders/water/water.frag");
    Water.MoveFactor = 0.f;
    Water.Quad       = CreateQuad();

    Water.ReflectHeight = HEIGHT / 1;
    Water.ReflectWidth  = WIDTH / 1;

    Water.RefractHeight = HEIGHT / 1;
    Water.RefractWidth  = WIDTH / 1;

    //-------------------Textures----------------------
    // clang-format off
    Water.Reflection = {"ReflectionTex", 0};
    Water.Refraction = {"RefractionTex", 1};
    Water.DUDV       = {"DUDV_Map",      2};
    Water.Normal     = {"NormalMap",     3};
    Water.DepthMap   = {"DepthMap",      4};

    LoadTexture("../../models/water/DUDV.png", &Water.DUDV);
    LoadTexture("../../models/water/normal.png", &Water.Normal);

    SetTextureUniform(Water.Reflection, Water.Shader);
    SetTextureUniform(Water.Refraction, Water.Shader);
    SetTextureUniform(Water.DUDV,       Water.Shader);
    SetTextureUniform(Water.Normal,     Water.Shader);
    SetTextureUniform(Water.DepthMap,   Water.Shader);
    // clang-format on

    //-------------------FBOs----------------------
    // ----------Reflection---------------
    Water.ReflectFBO    = CreateFBO();
    Water.Reflection.ID = ColorTextureAttachment(Water.ReflectWidth, Water.ReflectHeight);
    DepthBufferAttachment(Water.ReflectWidth, Water.ReflectHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // -----------Refraction--------------
    Water.RefractFBO    = CreateFBO();
    Water.Refraction.ID = ColorTextureAttachment(Water.RefractWidth, Water.RefractHeight);
    Water.DepthMap.ID   = DepthTextureAttachment(Water.RefractWidth, Water.RefractHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //----------------Transformations----------------------

    vec3 WaterTranslate = vec3(546, -255, 717);
    vec3 WaterRotate    = vec3(30, 270, 0);
    vec3 WaterScale     = vec3(285, 500, 1);

    Water.Height = WaterTranslate.y;

    mat4 WaterMatrix = mat4(1);
    WaterMatrix      = translate(WaterMatrix, WaterTranslate);
    WaterMatrix      = rotate(WaterMatrix, radians(WaterRotate.x), vec3(0, 1, 0));
    WaterMatrix      = rotate(WaterMatrix, radians(WaterRotate.y), vec3(1, 0, 0));
    WaterMatrix      = scale(WaterMatrix, WaterScale);

    Water.Tranformation = WaterMatrix;

    return Water;
};
