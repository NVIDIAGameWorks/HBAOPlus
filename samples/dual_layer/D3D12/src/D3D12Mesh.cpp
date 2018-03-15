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
#include "D3D12Mesh.h"
#include "d3dx12.h"

bool D3D12Mesh::InitializeFromMesh(ID3D12Device* device, Mesh& mesh)
{
    mNumIndices = (uint32_t)mesh.mIndices.size();
    
    ID3D12Resource* vertexBuffer = nullptr;
    ID3D12Resource* indexBuffer = nullptr;

    uint32_t vertexBufferSize = (uint32_t)mesh.mVertices.size() * sizeof(decltype(mesh.mVertices)::value_type);
    uint32_t indexBufferSize = (uint32_t)mesh.mIndices.size() * sizeof(decltype(mesh.mIndices)::value_type);

    device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer));

    device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer));

    void* uploadPtr = nullptr;
    vertexBuffer->Map(0, nullptr, &uploadPtr);
    std::copy(mesh.mVertices.begin(), mesh.mVertices.end(), (decltype(mesh.mVertices)::value_type*)uploadPtr);
    vertexBuffer->Unmap(0, nullptr);

    indexBuffer->Map(0, nullptr, &uploadPtr);
    std::copy(mesh.mIndices.begin(), mesh.mIndices.end(), (decltype(mesh.mIndices)::value_type*)uploadPtr);
    indexBuffer->Unmap(0, nullptr);

    mVertexBuffer.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    mVertexBuffer.SizeInBytes = vertexBufferSize;
    mVertexBuffer.StrideInBytes = sizeof(decltype(mesh.mVertices)::value_type);

    mIndexBuffer.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    mIndexBuffer.SizeInBytes = indexBufferSize;
    mIndexBuffer.Format = DXGI_FORMAT_R32_UINT;

    return true;
}
