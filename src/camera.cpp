//
// Created by AgentOfChaos on 4/5/2022.
//

#include "camera.h"

// Default camera values
// const r32 YAW         = -90.0f + 0.f;
const r32 YAW = 90.0f + 0.f;

const r32 PITCH       = -30.0f + 27.f;
const r32 SPEED       = 20.5f;
const r32 SENSITIVITY = 0.1f;
r32       FOV         = radians(70.0f);

mat4 GetViewMatrix(camera *Camera)
{
    return glm::lookAt(Camera->Position, Camera->LookAt, Camera->Up);
    // return glm::lookAt(camera->tpCamPos, camera->LookAt, camera->WorldUp);
};

camera
InitCamera(vec3 Position)
{
    camera camera   = {};
    camera.Position = Position;
    camera.Front    = vec3(0.f, 0.f, -1.f);

    camera.Yaw     = YAW;
    camera.Pitch   = PITCH;
    camera.WorldUp = vec3(0.0f, 1.0f, 0.0f);

    camera.MovementSpeed    = SPEED;
    camera.MouseSensitivity = SENSITIVITY;
    camera.FirstClick       = true;

    camera.DistanceOffset = vec3(0.f, 20.5f, 30.f);
    // camera.distanceOffset = vec3(0.f);

    // updateCameraVectors(&camera);

    return (camera);
}
