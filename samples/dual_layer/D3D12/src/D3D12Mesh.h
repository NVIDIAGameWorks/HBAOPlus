/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#pragma once
#include "Mesh.h"
#include <d3d12.h>
#include <DirectXMath.h>

using namespace DirectX;

class D3D12Mesh
{
public:
    bool InitializeFromMesh(ID3D12Device* device, Mesh& mesh);
    uint32_t                    mNumIndices;
    XMMATRIX                    mMatWorld;
    D3D12_VERTEX_BUFFER_VIEW    mVertexBuffer;
    D3D12_INDEX_BUFFER_VIEW     mIndexBuffer;
};

