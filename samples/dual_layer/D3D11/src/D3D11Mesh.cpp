/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#define _SCL_SECURE_NO_WARNINGS
#include "D3D11Mesh.h"

bool D3D11Mesh::InitializeFromMesh(ID3D11Device* device, Mesh& mesh)
{
    mNumIndices = (uint32_t)mesh.mIndices.size();
    
    uint32_t vertexBufferSize = (uint32_t)mesh.mVertices.size() * sizeof(decltype(mesh.mVertices)::value_type);
    uint32_t indexBufferSize = (uint32_t)mesh.mIndices.size() * sizeof(decltype(mesh.mIndices)::value_type);

    D3D11_BUFFER_DESC bufferDesc{};
    D3D11_SUBRESOURCE_DATA initialData{};

    bufferDesc.ByteWidth = vertexBufferSize;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    initialData.pSysMem = mesh.mVertices.data();
    initialData.SysMemPitch = 0;
    initialData.SysMemSlicePitch = 0;
    device->CreateBuffer(&bufferDesc, &initialData, &mVertexBuffer);


    bufferDesc.ByteWidth = indexBufferSize;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    initialData.pSysMem = mesh.mIndices.data();
    initialData.SysMemPitch = 0;
    initialData.SysMemSlicePitch = 0;
    device->CreateBuffer(&bufferDesc, &initialData, &mIndexBuffer);

    return true;
}
