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
#include <D3D11.h>
#include <wrl.h>
#include <vector>

using namespace Microsoft::WRL;

class Camera;
class D3D11Mesh;

class MeshRenderer
{
public:
    void Initialize(ID3D11Device* device);
    void RenderMeshes(ID3D11DeviceContext* deviceContext, const std::vector<D3D11Mesh>& meshes, const Camera& camera);

private:
    //ComPtr<ID3D12PipelineState> mPSO;
    //ComPtr<ID3D12RootSignature> mRootSignature;
    ComPtr<ID3D11InputLayout>       mInputLayout;
    ComPtr<ID3D11RasterizerState>   mRasterizerState;
    ComPtr<ID3D11BlendState>        mBlendState;
    ComPtr<ID3D11DepthStencilState> mDepthStencilState;

    ComPtr<ID3D11VertexShader>      mVertexShader;
    ComPtr<ID3D11PixelShader>       mPixelShader;

    ComPtr<ID3D11Buffer>            mPerFrameCB;
    ComPtr<ID3D11Buffer>            mPerObjectCB;
};