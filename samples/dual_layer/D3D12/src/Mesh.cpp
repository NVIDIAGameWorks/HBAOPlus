/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <Windows.h>

std::shared_ptr<Mesh> Mesh::LoadFromFile(const std::string& filename)
{
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    std::ifstream inputFile(filename);
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &inputFile, nullptr, true);

    if (shapes.size() == 0)
    {
        MessageBoxW(NULL, L"Mesh::LoadFromFile failed", L"Error!", MB_ICONEXCLAMATION | MB_OK);
    }

    tinyobj::shape_t& shape = shapes[0];
    

    for (auto& index : shape.mesh.indices)
    {
        mesh->mIndices.push_back((uint32_t)index.vertex_index);
    }

    Vertex vertex{};
    for (uint32_t i = 0; i < attrib.vertices.size() / 3; ++i)
    {
        memcpy(vertex.Position, &attrib.vertices[i * 3], sizeof(float) * 3);
        mesh->mVertices.push_back(vertex);
    }
    return mesh;
}