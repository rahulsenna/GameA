//
// Created by AgentOfChaos on 4/8/2022.
//

#include "input.h"
#include "physics.h"
#include "animation.h"

vec3 AddPlayerCam(0, 2, 0);
vec3 ADDCamPos(0, 0, 0);

b32 CamPosByTig         = false;
b32 InputDependent      = true;
b32 CamRotationByMatrix = true;

r32 LookAtX = 1.f;

extern PxController *MainCCT;

void UpdateCamera(camera *Camera)
{
    r32 t = 0.15f;
    if (InputDependent)
    {
        Camera->p = Camera->Pitch + 30.f;
        Camera->y = Camera->Yaw + 90.f;
    }

    r32 yaw   = glm::radians(Camera->y);
    r32 pitch = glm::radians(Camera->p);

    //    vec3 PlayerPos = Camera->target->Position + AddPlayerCam;
    vec3 PlayerPos = V3PxVec3(MainCCT->getPosition()) + AddPlayerCam;

    vec3 distanceOffset = Camera->DistanceOffset;

    // if (pitch < 0.f) { pitch = 0.f; }
    {
        glm::quat qx   = glm::angleAxis(yaw, glm::vec3(0.f, 1.f, 0.f));
        glm::quat qy   = glm::angleAxis(-pitch, glm::vec3(1.f, 0.f, 0.f));
        distanceOffset = distanceOffset * qy;
        distanceOffset = distanceOffset * qx;

        // mat4 m         = mat4(1.f);
        // m              = rotate(m, -yaw, vec3(0.f, 1.f, 0.f));
        // m              = rotate(m, pitch, vec3(1.f, 0.f, 0.f));
        // distanceOffset = vec3(m * vec4(distanceOffset, 1.f));
    }
    distanceOffset = PlayerPos + distanceOffset;

    //    if (Camera->target->AnimState != Fall)
    //    {
    //        distanceOffset = PlayerPos + distanceOffset;
    //    } else
    //    {
    //        distanceOffset.z = PlayerPos.z;
    //    }
    // camera->tpCamPos = glm::mix(camera->tpCamPos+ADDCamPos, distanceOffset, t);
    Camera->ThirdPCamPos = ADDCamPos + distanceOffset;

    Camera->Front = glm::normalize((PlayerPos - Camera->ThirdPCamPos) * LookAtX);
    Camera->Right = glm::normalize(glm::cross(Camera->Front, Camera->WorldUp));

    Camera->Up     = glm::normalize(glm::cross(Camera->Right, Camera->Front));
    Camera->LookAt = PlayerPos + Camera->Front;

    // camera->Up     = glm::mix(camera->Up, glm::normalize(glm::cross(camera->Right, camera->Front)), t);
    // camera->LookAt = glm::mix(camera->LookAt,  PlayerPos + camera->Front, t);

#if 0
    if (InputDependent)
        {
            camera->p = -camera->Pitch;
            camera->y = camera->Yaw - 90.0f;
        }

        r32 pitch = glm::radians(camera->p);
        r32 yaw   = glm::radians(camera->y);

        if (pitch < 00.0) { pitch = 00.0; }

        if (CamPosByTig)
        {
            r32 distance           = 5.0f;
            r32 horizontalDistance = distance * cos(pitch);
            r32 verticalDistance   = distance * sin(pitch);

            r32 x = horizontalDistance * sin(-yaw);
            r32 y = verticalDistance;
            r32 z = horizontalDistance * cos(yaw);

            vec3 TargetPos = camera->target->Position + glm::vec3(-x, y, -z);

            camera->tpCamPos = TargetPos;


        }

#endif
}

static s32 scroll;

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    scroll = (s32)yoffset;
}

r32 LastMouseX, LastMouseY = 0.f;
r32 camYaw;

extern PxScene *ScenePhysX;

PxObstacleContext        *ObstacleContext   = NULL;
const PxControllerFilters ControllerFilters = PxControllerFilters();

extern player Player;

void HandleMouseInputs(camera *Camera, GLFWwindow *Window)
{

    // Mouse
    if (glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        r64 mouseX, mouseY;
        glfwGetCursorPos(Window, &mouseX, &mouseY);

        if (Camera->FirstClick)
        {
            LastMouseX         = (r32)mouseX;
            LastMouseY         = (r32)mouseY;
            Camera->FirstClick = false;
        }

        r32 xoffset = mouseX - LastMouseX;
        r32 yoffset = LastMouseY - (r32)mouseY;// reversed since y-coordinates go from bottom to top

        LastMouseX = mouseX;
        LastMouseY = (r32)mouseY;

        if (mouseX > (r32) WIDTH)
        {
            glfwSetCursorPos(Window, (r32) WIDTH, mouseY);
            LastMouseX = (r32) WIDTH;
        }
        // if (mouseY > (r32) HEIGHT)
        // {
        //     glfwSetCursorPos(Window, mouseX, (r32) HEIGHT);
        //     LastMouseX = (r32) HEIGHT;
        // }

        xoffset *= Camera->MouseSensitivity;
        yoffset *= Camera->MouseSensitivity;
        Camera->Yaw += xoffset;
        Camera->Pitch += yoffset;
    }

    if ((glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) &&
        glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        Camera->FirstClick = true;
    }
}

void HandleKeyInputs(camera *Camera, GLFWwindow *Window, r32 dt)
{
    r32 velocity = dt * Camera->MovementSpeed * 12.f;

    if (scroll == -1)
    {
        // Zoom out
        // camera->distanceOffset/= 0.9;
        Camera->DistanceOffset.z += .3;
        // camera->Pitch -= 3;

        scroll = 0;
    }
    if (scroll == 1)
    {
        // camera->distanceOffset*= 0.9;
        Camera->DistanceOffset.z -= .3;
        // camera->Pitch += 3;

        scroll = 0;
    }

    if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(Window, GL_TRUE);
    }

    if (glfwGetKey(Window, GLFW_KEY_J) == GLFW_PRESS)
    {
        Camera->Position += velocity * -glm::normalize(glm::cross(Camera->Front, Camera->Up));
    }
    if (glfwGetKey(Window, GLFW_KEY_L) == GLFW_PRESS)
    {
        Camera->Position += velocity * glm::normalize(glm::cross(Camera->Front, Camera->Up));
    }
    if (glfwGetKey(Window, GLFW_KEY_I) == GLFW_PRESS)
    {
        Camera->Position += velocity * Camera->WorldUp;
    }
    if (glfwGetKey(Window, GLFW_KEY_K) == GLFW_PRESS)
    {
        Camera->Position += velocity * -Camera->WorldUp;
    }

    if (glfwGetKey(Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        Camera->MovementSpeed = 6.f;
    }

    if (glfwGetKey(Window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
    {
        Camera->MovementSpeed = 1.f;
    }

    PxVec3 Displacement = ScenePhysX->getGravity() * dt * 12;
    //Walk
    if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
    {
        Displacement.x = velocity * Camera->Front.x;
        Displacement.z = velocity * Camera->Front.z;

        // camera->target->Position += velocity * glm::vec3(camera->Front.x, 0.f, camera->Front.z);
        r32 A = -Camera->y;

        if ((A - Player.RotateX) > 360 || (A - Player.RotateX) < -360)
        {
            Camera->Yaw    = ((int) Camera->Yaw % 360);
            Player.RotateX = ((int) Player.RotateX % 360);
        };

        if (glfwGetKey(Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            Player.AnimState = Run;
            Player.RotateX   = glm::mix(Player.RotateX, A, 0.05f);
        }

        if (glfwGetKey(Window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        {
            Player.AnimState = Walk;
            Player.RotateX   = glm::mix(Player.RotateX, A, 0.01f);
        }
    }
    if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
    {
        Displacement.x   = -velocity * Camera->Front.x;
        Displacement.z   = -velocity * Camera->Front.z;
        Player.AnimState = WalkBack;
    }

    if ((glfwGetKey(Window, GLFW_KEY_W) == GLFW_RELEASE) &&
        (glfwGetKey(Window, GLFW_KEY_S) == GLFW_RELEASE))
    {
        Player.AnimState = Idle;
    }

    MainCCT->move(Displacement, 0.001f, dt, ControllerFilters, ObstacleContext);
}
