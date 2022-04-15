#pragma once

// Include GLEW
// #include <GL/glew.h>
// #include <glad/glad.h>

#include "platform.h"

struct shape
{
    u32 VAO;
    u32 numIndices;
};

shape CreateQuad();

shape createBox(float w, float h, float l);

shape createPlane(float base, float size, float uvScale);

shape createSphere(float rad, u32 hSegs, u32 vSegs);

shape CreateBoxMinMax(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);

shape CreateCubeTextured(r32 w, r32 h, r32 l);

shape CreateFrustum(std::vector<vec4> &f);

void RenderShape(shape *shape);

void RenderSide(shape *shape, unsigned int Side);
