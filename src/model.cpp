//
// Created by AgentOfChaos on 4/8/2022.
//

#include "model.h"
#include "mesh.h"
#include <setjmp.h>

// #define FAST_MODEL

void SetVertexBoneDataToDefault(Vertex &vertex)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        vertex.m_BoneIDs[i] = -1;
        vertex.m_Weights[i] = 0.0f;
    }
}

void SetVertexBoneData(Vertex &vertex, int boneID, float weight)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        if (vertex.m_BoneIDs[i] < 0)
        {
            vertex.m_Weights[i] = weight;
            vertex.m_BoneIDs[i] = boneID;
            break;
        }
    }
}

void ExtractBoneWeightForVertices(model *model, std::vector<Vertex> &vertices, aiMesh *mesh, const aiScene *scene)
{
    auto &boneInfoMap = model->m_BoneInfoMap;
    int  &boneCount   = model->m_BoneCounter;

    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
        int         boneID   = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            BoneInfo newBoneInfo;
            newBoneInfo.id        = boneCount;
            newBoneInfo.offset    = ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
            boneInfoMap[boneName] = newBoneInfo;
            boneID                = boneCount;
            boneCount++;
        } else
        {
            boneID = boneInfoMap[boneName].id;
        }
        assert(boneID != -1);
        auto weights    = mesh->mBones[boneIndex]->mWeights;
        int  numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            int   vertexId = weights[weightIndex].mVertexId;
            float weight   = weights[weightIndex].mWeight;
            assert(vertexId <= vertices.size());
            SetVertexBoneData(vertices[vertexId], boneID, weight);
        }
    }
}

void ExtractBone1(model *model, aiMesh *mesh)
{
    auto &boneInfoMap = model->m_BoneInfoMap;
    int  &boneCount   = model->m_BoneCounter;

    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
        int         boneID   = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            BoneInfo newBoneInfo;
            newBoneInfo.id        = boneCount;
            newBoneInfo.offset    = ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
            boneInfoMap[boneName] = newBoneInfo;
            boneID                = boneCount;
            boneCount++;
        }
    }
}

static std::vector<texture>
LoadMaterials(model *model, const aiScene *scene, aiMaterial *material, aiTextureType type, const char *typeName)
{
    std::vector<texture> textures;

    for (unsigned int i = 0; i < material->GetTextureCount(type); ++i)
    {
        aiString texPath;
        material->GetTexture(type, i, &texPath);

        bool skip = false;
        for (unsigned int j = 0; j < model->loadedTexName.size(); j++)
        {
            if (model->loadedTexName[j] == texPath.C_Str())
            {
                textures.push_back(model->loadedTex[j]);
                skip = true;
                break;
            }
        }

        // If the texture has been loaded, skip this
        if (!skip)
        {
            texture tex = {};
            tex.type    = typeName;
            tex.unit    = model->loadedTex.size() + 1;

            if (auto texture = scene->GetEmbeddedTexture(texPath.C_Str()))
            {
                LoadTexture(texture, &tex);

            } else
            {
                std::string dir = model->file.substr(0, model->file.find_last_of('/') + 1);
                LoadTexture((dir + texPath.C_Str()).c_str(), &tex);
            }
            textures.push_back(tex);
            model->loadedTex.push_back(tex);
            model->loadedTexName.push_back(texPath.C_Str());
            cout << "unit: " << tex.unit << " id: " << tex.ID << " tex.type: " << tex.type << " - " << texPath.C_Str() << endl;
        }
    }
    return textures;
}

static void
parse_single_bone(unsigned int bone_index, aiBone *bone)
{
    // printf("Bone %d: '%s' num vertices affected by this bone: %d\n", bone_index, bone->mName.C_Str(), bone->mNumWeights);
    for (int i = 0; i < bone->mNumWeights; ++i)
    {
        aiVertexWeight vw = bone->mWeights[i];
    }
}

static void
parse_mesh_bones(aiMesh *mesh)
{
    for (unsigned int i = 0; i < mesh->mNumBones; ++i)
    {
        parse_single_bone(i, mesh->mBones[i]);
    }
}

static void
processMesh(model *model, aiMesh *mesh, const aiScene *scene, u32 meshCount)
{
    // if (scene->HasMaterials())

    std::vector<texture> textures;
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        // 1. diffuse maps
        std::vector<texture> diffuseMaps = LoadMaterials(model, scene, material, aiTextureType_DIFFUSE, "diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
#if 1

        // 2. specular maps
        std::vector<texture> specularMaps = LoadMaterials(model, scene, material, aiTextureType_SPECULAR, "specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        std::vector<texture> glossyMaps = LoadMaterials(model, scene, material, aiTextureType_SHININESS, "glossy");
        textures.insert(textures.end(), glossyMaps.begin(), glossyMaps.end());

        std::vector<texture> opacityMaps = LoadMaterials(model, scene, material, aiTextureType_OPACITY, "opacity");
        textures.insert(textures.end(), opacityMaps.begin(), opacityMaps.end());

        // 3. normal maps
        std::vector<texture> normalMaps = LoadMaterials(model, scene, material, aiTextureType_NORMALS, "normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. normal maps
        std::vector<texture> heightMaps = LoadMaterials(model, scene, material, aiTextureType_HEIGHT, "normal");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        std::vector<texture> displacementMaps = LoadMaterials(model, scene, material, aiTextureType_DISPLACEMENT, "displacement");
        textures.insert(textures.end(), displacementMaps.begin(), displacementMaps.end());
#endif
    }

#ifndef FAST_MODEL
    std::vector<Vertex> vertices;
    std::vector<u32>    indices;
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex = {};
        SetVertexBoneDataToDefault(vertex);

        // vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);

        vertex.position = glm::vec3(
        mesh->mVertices[i].x,
        mesh->mVertices[i].y,
        mesh->mVertices[i].z);

        vertex.normal = glm::vec3(
        mesh->mNormals[i].x,
        mesh->mNormals[i].y,
        mesh->mNormals[i].z);

        if (mesh->HasTangentsAndBitangents())
        {
            vertex.tangent = glm::vec3(
            mesh->mTangents[i].x,
            mesh->mTangents[i].y,
            mesh->mTangents[i].z);

            vertex.biTangent = glm::vec3(
            mesh->mBitangents[i].x,
            mesh->mBitangents[i].y,
            mesh->mBitangents[i].z);
        }

        if (mesh->HasTextureCoords(0))
        {
            vertex.texUV = glm::vec2(
            mesh->mTextureCoords[0][i].x,
            mesh->mTextureCoords[0][i].y);
        } else
        {
            vertex.texUV = glm::vec2(0.f);
        }
        vertices.push_back(vertex);
    }

    // indices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }
    ExtractBoneWeightForVertices(model, vertices, mesh, scene);

    std::string dir = model->file.substr(0, model->file.find_last_of('/') + 1) + "verts/";

    // WriteToFile(string(dir+ std::to_string(meshCount) + ".verts").c_str(),
    // vertices.data(), vertices.size() * sizeof(Vertex));
    // WriteToFile(string(dir + std::to_string(meshCount) + "inds").c_str(),
    // indices.data(), indices.size() * sizeof(GLuint));

#endif
    /*
    Don't know what is this doing

    if (mesh->HasBones())
    {
        parse_mesh_bones(mesh);
    }
*/

#ifdef FAST_MODEL

    float     BeforeTime = glfwGetTime();
    mesh_data Data       = {};
    Data.Vertices        = ReadFromFile(string(model->file + ".verts." + std::to_string(meshCount)).c_str());
    Data.Indices         = ReadFromFile(string(model->file + ".inds." + std::to_string(meshCount)).c_str());
    Data.textures        = textures;
    SetupMesh(&Data);
    model->MeshDATA.push_back(Data);
    float AfterTime = glfwGetTime();

    cout << "DataLoadTime: " << AfterTime - BeforeTime << endl;

    ExtractBone1(model, mesh);
#else
    model->meshes.push_back(SetupMesh(vertices, indices, textures));
#endif
}

void processNode(model *model, aiNode *node, const aiScene *scene, u32 count)
{
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        // cout << "FILE: " << model->file << " MeshFound: " << endl;
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(model, mesh, scene, count);

        if (!node->mTransformation.IsIdentity())
        {

            model->transform = ConvertMatrixToGLMFormat(node->mTransformation);

            aiVector3D position, scaling, rotation;
            node->mTransformation.Decompose(scaling, rotation, position);
            model->rotation = glmV3fromAiV3(rotation);
            model->scaling  = glmV3fromAiV3(scaling);
            model->position = glmV3fromAiV3(position);

            // cout << "--------" << pFile << "---------" << endl;
            cout << "rotation: " << glm::to_string(model->rotation) << endl;
            cout << "scaling: " << glm::to_string(model->scaling) << endl;
            cout << "position: " << glm::to_string(model->position) << endl;
        }
    }

    // process all child nodes recursivly
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        processNode(model, node->mChildren[i], scene, i);
    }
}

model AssimpLoadModel(const char *File)
{
    const aiScene *scene = 0;
    scene                = aiImportFile(File,
                                        // aiProcess_GenSmoothNormals |
                                        // aiProcess_FlipUVs |
                                        // aiProcess_JoinIdenticalVertices |
                                        aiProcess_CalcTangentSpace |
                                        aiProcess_Triangulate);

    // If the import failed, report it
    if (NULL == scene)
    {
        printf("assimp load error %s\n", aiGetErrorString());
    }

    model Model = {};
    Model.file  = File;

    cout << File << endl;

    Model.transform = ConvertMatrixToGLMFormat(scene->mRootNode->mTransformation);

    if (!scene->mRootNode->mTransformation.IsIdentity())
    {
        aiVector3D position, scaling, rotation;

        scene->mRootNode->mTransformation.Decompose(scaling, rotation, position);
        Model.rotation = glmV3fromAiV3(rotation);
        Model.scaling  = glmV3fromAiV3(scaling);
        Model.position = glmV3fromAiV3(position);

        // cout << "--------" << pFile << "---------" << endl;
        cout << "rotation: " << glm::to_string(Model.rotation) << endl;
        cout << "scaling: " << glm::to_string(Model.scaling) << endl;
        cout << "position: " << glm::to_string(Model.position) << endl;
    }

    // Now we can access the file's contents

    processNode(&Model, scene->mRootNode, scene, 0);

    // We're done. Release all resources associated with this import
    aiReleaseImport(scene);

    return Model;
}

void DrawModel(model *model, u32 shader)
{
#ifdef FAST_MODEL
    // Go over all meshes and draw each one
    for (unsigned int i = 0; i < model->MeshDATA.size(); i++)
    {
        drawMesh(&model->MeshDATA[i], shader);
    }
#else
    for (unsigned int i = 0; i < model->meshes.size(); i++)
    {
        drawMesh(&model->meshes[i], shader);
    }
#endif
}
