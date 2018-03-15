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
#include <D3D12.h>
#include <wrl.h>
#include <vector>

using namespace Microsoft::WRL;

class Camera;
class D3D12Mesh;

class MeshRenderer
{
public:
    void Initialize(ID3D12Device* device);
    void RenderMeshes(ID3D12GraphicsCommandList* commandList, const std::vector<D3D12Mesh>& meshes, const Camera& camera);

private:
    ComPtr<ID3D12PipelineState> mPSO;
    ComPtr<ID3D12RootSignature> mRootSignature;
};