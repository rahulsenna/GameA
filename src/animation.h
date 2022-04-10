//
// Created by AgentOfChaos on 4/8/2022.
//

#pragma once

#include "platform.h"

#include "model.h"

enum AnimationState
{
    Idle,
    Walk,
    WalkBack,
    Jog,
    Run,
    Jump,
    Fall
};

struct AssimpNodeData
{
    mat4                        transformation;
    std::string                 name;
    s32                         childrenCount;
    std::vector<AssimpNodeData> children;
};

struct Animation
{
    r32                             Duration;
    s32                             TicksPerSecond;
    std::vector<Bone>               Bones;
    AssimpNodeData                  RootNode;
    std::map<std::string, BoneInfo> BoneInfoMap;

    //For blending animations
    r32            blendFactor;
    AnimationState state;
};

struct Animator
{
    std::vector<mat4> FinalBoneMatrices;
    Animation         *CurrentAnimation;
    r32               CurrentTime;
    r32               dt;
};

struct AnimManager
{
    AnimationState state;
    Animation      animation;
    Animator       animator;
};

struct player
{
    vec3 Position;
    vec3 Front;

    r32 RotateX;

    AnimationState AnimState;
    Animation *currentAnim;
    Animation *prevAnim;
};


void CrossFadeAnimation(player *Player, Animator *animator, Animation *anim, r32 dt);
AnimManager MakeAnimationManager(model *model, const char *Filename, AnimationState state);
