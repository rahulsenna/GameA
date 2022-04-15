//
// Created by AgentOfChaos on 4/8/2022.
//

#include "mesh.h"
#include "shader.h"

void SetupMesh(mesh_data *Data)
{
    Data->VAO  = createVAO();
    u32 VBO = createVBO(Data->Vertices);
    u32 EBO = createEBO(Data->Indices);

    linkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void *) 0);
    linkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void *) offsetof(Vertex, normal));
    linkAttrib(VBO, 2, 2, GL_FLOAT, sizeof(Vertex), (void *) offsetof(Vertex, texUV));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void *) offsetof(Vertex, m_BoneIDs));
    // weights
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, m_Weights));
    linkAttrib(VBO, 5, 3, GL_FLOAT, sizeof(Vertex), (void *) offsetof(Vertex, tangent));
    linkAttrib(VBO, 6, 3, GL_FLOAT, sizeof(Vertex), (void *) offsetof(Vertex, biTangent));

    unbindVAO();
    unbindVBO();
    unbindEBO();
}

Mesh SetupMesh(std::vector<Vertex> &vertices, std::vector<u32> &indices, std::vector<texture> &texture)
{
    Mesh mesh     = {};
    mesh.indices  = indices;
    mesh.vertices = vertices;
    mesh.textures = texture;

    mesh.VAO   = createVAO();
    u32 VBO = createVBO(vertices);
    u32 EBO = createEBO(indices);

    linkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void *) 0);
    linkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void *) offsetof(Vertex, normal));
    linkAttrib(VBO, 2, 2, GL_FLOAT, sizeof(Vertex), (void *) offsetof(Vertex, texUV));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void *) offsetof(Vertex, m_BoneIDs));
    // weights
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, m_Weights));
    linkAttrib(VBO, 5, 3, GL_FLOAT, sizeof(Vertex), (void *) offsetof(Vertex, tangent));
    linkAttrib(VBO, 6, 3, GL_FLOAT, sizeof(Vertex), (void *) offsetof(Vertex, biTangent));

    unbindVAO();
    unbindVBO();
    unbindEBO();

    return mesh;
}

void DrawMeshGeneric(u32 ShaderID, u32 VAO, std::vector<texture> &textures, u32 IndicesSize)
{
    // cout << "mesh VAO: " << VAO << endl;
    glUseProgram(ShaderID);
    bindVAO(VAO);

    u32 numDiffuse  = 0;
    u32 numSpecular = 0;
    u32 numNormal   = 0;
    u32 numHeight   = 0;
    u32 numGlossy   = 0;
    u32 numOpacity  = 0;

    for (u32 i = 0; i < textures.size(); i++)
    {
        std::string num;
        std::string type = textures[i].type;

        if (type == "diffuse")
        {
            num = std::to_string(numDiffuse++);
        } else if (type == "specular")
        {
            num = std::to_string(numSpecular++);
        } else if (type == "normal")
        {
            num = std::to_string(numNormal++);
        } else if (type == "height")
        {
            num = std::to_string(numHeight++);
        } else if (type == "glossy")
        {
            num = std::to_string(numGlossy++);
        } else if (type == "opacity")
        {
            num = std::to_string(numOpacity++);
        }

        SetTextUniform((type + num).c_str(), textures[i].unit, ShaderID);
        BindTexture(textures[i]);
    }

    glDrawElements(GL_TRIANGLES, IndicesSize, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void drawMesh(mesh_data *mesh, u32 shader)
{
    DrawMeshGeneric(shader, mesh->VAO, mesh->textures, mesh->Indices.size / sizeof(u32));
}

void drawMesh(Mesh *mesh, u32 shader)
{
    DrawMeshGeneric(shader, mesh->VAO, mesh->textures, mesh->indices.size());
}
