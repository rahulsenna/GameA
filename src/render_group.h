//
// Created by AgentOfChaos on 4/8/2022.
//

#pragma once

#include "platform.h"
#include "basic_shapes.h"
#include "physics.h"
#include "camera.h"
#include "model.h"

#include "animation.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct render_group_shape
{
    u32 ShaderID;
    u32 ShadowShaderID;

    std::vector<shape>   Shapes;
    std::vector<texture> Textures;

    std::vector<vec4>             Colors;
    std::vector<PxRigidDynamic *> PhysXColliders;
};

struct dynamic_model_render_group
{
    u32                ShaderID;
    u32                ShadowShaderID;
    std::vector<model> Models;

    std::vector<mat4>                                  Transforms;
    std::vector<std::vector<mat4>>                     AnimTransforms;
    std::vector<std::map<AnimationState, AnimManager>> AnimManager;
};

struct static_model_render_group
{
    u32                ShaderID;
    u32                ShadowShaderID;
    std::vector<model> Models;
    std::vector<mat4>  Transforms;
    std::vector<u32>   DebugVAO;
    std::vector<u32>   DebugIndicesCount;
};
struct shadow_map
{
    s32 Width;
    s32 Height;
    u32 DepthMap;
    u32 FBO;
};

struct debug_camera
{
    b32 Enable;
    b32 EnableShadowBox;

    shape Sun;

    vec3 Pos;
    vec2 Rot;
    r32  Zoom;

    mat4              LightView;
    std::vector<vec4> Frustum;
    vec3              Min, Max;
};

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

void InitImGUI(GLFWwindow *Window);

void RenderImGui(r32 dt);


mat4 CalculateLightSpaceMatrix(camera *Camera);

void RenderShapes(render_group_shape *Group, u32 ShaderID);

void RenderAnimatedModels(dynamic_model_render_group *Group, u32 ShaderID);

void RenderPassShadow(mat4 &LightSpaceMatrix, render_group_shape &GroupShape, dynamic_model_render_group &DynamicModelRenderGroup, shadow_map &ShadowMap);

shadow_map SetupShadowMapTexture(s32 Width, s32 Height);

mat4 DebugViewMatrix(debug_camera *DebugCamera);

void RenderSun(u32 &ShaderID, mat4 &Projection, mat4 &View, vec3 &Pos);

void RenderDebugCamera(u32 &ShaderID, mat4 &Projection, mat4 &View);

void PhysXDebugRender(u32 &ShaderID, mat4 &Projection, mat4 &View);
