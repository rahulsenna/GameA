#include <GL/glew.h>
//#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION

#include "stb/stb_image.h"

#include "platform.h"

#include "game.h"
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);


    GLFWwindow *Window = glfwCreateWindow(WIDTH, HEIGHT, "Learn OpenGL", 0, 0);
    if (Window == NULL)
    {
        std::cout << "Failed to create GLFW Window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(Window);

    GLFWmonitor *monitor = glfwGetWindowMonitor(Window);
    // glfwSetWindowMonitor(window, monitor, 0, 0, 2560, 1440, 240);
    // glfwSetWindowSize(Window, 2560, 1440);
    glfwSetWindowSize(Window, 2560, HEIGHT);

    glfwSetWindowPos(Window, 5, 70);

    glfwSetScrollCallback(Window, scroll_callback);

    GLFWimage Icon = {};
    int       NumColCh;
    stbi_set_flip_vertically_on_load(true);

    const char *IconFile = "../../models/textures/right.png";
    Icon.pixels    = stbi_load(IconFile, &Icon.width, &Icon.height, &NumColCh, 0);

    glfwSetWindowIcon(Window, 1, &Icon);

    //    gladLoadGL();
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Error!" << std::endl;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthFunc(GL_LESS);

#ifdef FACE_CULLING
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);
    // glFrontFace(GL_CCW);
#endif
    glLineWidth(3);

    InitGame(Window);

    while (!glfwWindowShouldClose(Window))
    {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        UpdateAndRender(Window);

        glfwSwapBuffers(Window);
        glfwPollEvents();
    }

    glfwDestroyWindow(Window);
    glfwTerminate();

    return 0;
}
