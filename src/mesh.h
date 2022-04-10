//
// Created by AgentOfChaos on 4/8/2022.
//

#pragma once

#include "platform.h"
#include "model.h"

void SetupMesh(mesh_data *Data);
Mesh SetupMesh(std::vector<Vertex> &vertices, std::vector<u32> &indices, std::vector<texture> &texture);
void DrawMeshGeneric(u32 ShaderID, u32 VAO, std::vector<texture> &textures, u32 IndicesSize);
void drawMesh(mesh_data *mesh, u32 shader);
void drawMesh(Mesh *mesh, u32 shader);