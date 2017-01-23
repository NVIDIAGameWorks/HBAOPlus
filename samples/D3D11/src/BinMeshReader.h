/* 
* Copyright (c) 2008-2017, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#pragma once
#include <DirectXMath.h>

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 textureCoordinate;
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

FILE *OpenFile(std::string Path, const char* Mode)
{
    FILE *fp = NULL;
    if (fopen_s(&fp, Path.c_str(), Mode) || !fp)
    {
        MessageBox(NULL, L"Failed to open data file", L"Error", MB_OK | MB_ICONERROR);
        exit(1);
    }
    return fp;
}

bool LoadVertices(const char* FileName, std::vector<Vertex>& OutVertices)
{
    FILE* fp = OpenFile(FileName, "rb");

    fseek(fp, 0L, SEEK_END);
    UINT FileSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    UINT NumVertices = FileSize / sizeof(Vertex::position);

    for (UINT Idx = 0; Idx < NumVertices; ++Idx)
    {
        Vertex vertex;
        fread(&vertex.position, sizeof(DirectX::XMFLOAT3), 1, fp);
        vertex.normal.x = vertex.normal.y = 0;
        vertex.normal.z = 1.0;
        vertex.textureCoordinate.x = vertex.textureCoordinate.y = 0;
        OutVertices.push_back(vertex);
    }

    fclose(fp);
    return true;
}

bool LoadIndices(const char* FileName, std::vector<uint32_t>& OutIndices)
{
    FILE* fp = OpenFile(FileName, "rb");

    fseek(fp, 0L, SEEK_END);
    UINT FileSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    UINT NumIndices = FileSize / sizeof(uint32_t);

    for (UINT Idx = 0; Idx < NumIndices; ++Idx)
    {
        uint32_t index;
        fread(&index, sizeof(index), 1, fp);
        OutIndices.push_back(index);
    }

    fclose(fp);
    return true;
}
