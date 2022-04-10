#pragma once

#include <stdint.h>
#include <float.h>

#define _internal      static
#define _global        static
#define _local_persist static
#define PII32          3.1415926535897932f
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef bool    b32;

typedef float  r32;
typedef double r64;

_global const int WIDTH         = 1920;
_global const int HEIGHT        = 1080;
_global b32 GlobalRunning = true;

#define FACE_CULLING

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <string>
#include <vector>
#include <iostream>

using namespace glm;
using std::string, std::cout, std::endl;

struct texture
{
    const char *type;
    s32        unit;
    u32        ID;
};

#define MAX_BONE_INFLUENCE 4
#define MAX_BONES          200

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texUV;

    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    r32 m_Weights[MAX_BONE_INFLUENCE];

    glm::vec3 tangent;
    glm::vec3 biTangent;
};

struct FileData
{
    size_t size;
    void   *data;
};

_internal void WriteToFile(const char *Filename, void *Data, size_t Size)
{
    FILE *fptr;
    fptr = fopen(Filename, "wb");
    fwrite(Data, 1, Size, fptr);
    fclose(fptr);
}

_internal FileData ReadFromFile(const char *Filename)
{
    FileData file;

    FILE *fp = fopen(Filename, "rb");
    fseek(fp, 0L, SEEK_END);
    file.size = ftell(fp);
    file.data = malloc(file.size);
    fseek(fp, 0L, SEEK_SET);
    fread(file.data, 1, file.size, fp);
    fclose(fp);

    return file;
}

inline r32
Lerp(r32 A, r32 t, r32 B)
{
    r32 Result = (1.0f - t) * A + t * B;
    return (Result);
}

_internal glm::vec3
Lerp(glm::vec3 x, glm::vec3 y, r32 t)
{
    return x * (1.f - t) + y * t;
}
