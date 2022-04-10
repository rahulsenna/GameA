//
// Created by AgentOfChaos on 4/5/2022.
//

#pragma once

#include "platform.h"


struct camera
{
    vec3 Position;

    vec3 Front;
    vec3 Up;
    vec3 Right;

    vec3 WorldUp;
    vec3 LookAt;

    // euler Angles
    r32 Yaw;
    r32 Pitch;

    r32 y;
    r32 p;

    r32 MovementSpeed, MouseSensitivity;
    b32 FirstClick;

    vec3 DistanceOffset;

//    Player   *target;
    vec3 ThirdPCamPos;

    // float tYaw;
};

camera InitCamera(vec3 Position);

mat4 GetViewMatrix(camera *Camera);

void UpdateCamera(camera *Camera);

