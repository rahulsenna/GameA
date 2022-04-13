//
// Created by AgentOfChaos on 4/8/2022.
//

#pragma once

#include "platform.h"
#include "camera.h"

#include <GL/glew.h>
#include "GLFW/glfw3.h"

void HandleKeyInputs(camera *Camera, GLFWwindow *Window, r32 dt);
void HandleMouseInputs(camera *Camera, GLFWwindow *Window);
