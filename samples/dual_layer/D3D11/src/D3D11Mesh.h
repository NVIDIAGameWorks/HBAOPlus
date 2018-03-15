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
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

using namespace DirectX;
using namespace Microsoft::WRL;

class D3D11Mesh
{
public:
    bool InitializeFromMesh(ID3D11Device* device, Mesh& mesh);
    uint32_t                    mNumIndices;
    XMMATRIX                    mMatWorld;
    ComPtr<ID3D11Buffer>        mVertexBuffer;
    ComPtr<ID3D11Buffer>        mIndexBuffer;
};

