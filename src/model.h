//
// Created by AgentOfChaos on 4/8/2022.
//

#pragma once

#include "platform.h"
#include <map>

#include <assimp/cimport.h>    // Plain-C interface
#include <assimp/scene.h>      // Output data structure
#include <assimp/postprocess.h>// Post processing flags

#include "shader.h"

struct mesh_data
{
    FileData Vertices;
    FileData Indices;

    u32                  VAO;
    std::vector<texture> textures;
    u32                  IndicesCount;
};
struct Mesh
{
    std::vector<Vertex>  vertices;
    std::vector<u32>     indices;
    std::vector<texture> textures;

    u32 VAO;
};

struct BoneInfo
{
    /*id is index in finalBoneMatrices*/
    int id;

    /*offset matrix transforms vertex from model space to bone space*/
    glm::mat4 offset;
};

struct model
{
    std::string                file;
    std::vector<unsigned char> data;


    std::map<std::string, BoneInfo> m_BoneInfoMap;
    int                             m_BoneCounter = 0;
    std::vector<Mesh>      meshes;

    // All the meshes and transformations
    // std::vector<glm::vec3> translationsMeshes;
    // std::vector<glm::quat> rotationsMeshes;
    // std::vector<glm::vec3> scalesMeshes;
    // std::vector<glm::mat4> matricesMeshes;

    // Prevents textures from being loaded twice
    std::vector<std::string> loadedTexName;
    std::vector<texture>     loadedTex;

    std::vector<mesh_data> MeshDATA;

    vec3 rotation;
    vec3 scaling ;
    vec3 position;

    mat4 transform;

};

struct KeyPosition
{
    glm::vec3 position;
    float     timeStamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float     timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float     timeStamp;
};

struct Bone
{
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale>    m_Scales;
    int                      m_NumPositions;
    int                      m_NumRotations;
    int                      m_NumScalings;

    glm::mat4   m_LocalTransform;
    std::string m_Name;
    int         m_ID;
};

static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4 &from)
{
    glm::mat4 to;
    //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    to[0][0] = from.a1;
    to[1][0] = from.a2;
    to[2][0] = from.a3;
    to[3][0] = from.a4;
    to[0][1] = from.b1;
    to[1][1] = from.b2;
    to[2][1] = from.b3;
    to[3][1] = from.b4;
    to[0][2] = from.c1;
    to[1][2] = from.c2;
    to[2][2] = from.c3;
    to[3][2] = from.c4;
    to[0][3] = from.d1;
    to[1][3] = from.d2;
    to[2][3] = from.d3;
    to[3][3] = from.d4;
    return to;
}

static inline glm::vec3 glmV3fromAiV3(const aiVector3D &vec)
{
    return glm::vec3(vec.x, vec.y, vec.z);
}

static inline glm::quat GetGLMQuat(const aiQuaternion &pOrientation)
{
    return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
}

static void
LoadTexture(const aiTexture *Data, texture *texture)
{
    int imgWidth, imgHeight, numColCh;
    stbi_set_flip_vertically_on_load(true);

    unsigned char *bytes = stbi_load_from_memory((const unsigned char *) Data->pcData,
                                                 Data->mWidth, &imgWidth, &imgHeight, &numColCh, 0);

    UploadTexture(texture, bytes, imgWidth, imgHeight, numColCh);
}

model AssimpLoadModel(const char *pFile);
void DrawModel(model *model, u32 shader);