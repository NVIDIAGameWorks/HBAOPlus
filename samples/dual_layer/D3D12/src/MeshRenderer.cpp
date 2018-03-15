/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "MeshRenderer.h"
#include "d3dx12.h"
#include <d3dcompiler.h>

#include "D3D12Mesh.h"
#include <Camera.h>

void MeshRenderer::Initialize(ID3D12Device* device)
{
    CD3DX12_ROOT_PARAMETER rootParameters[2];
    rootParameters[0].InitAsConstants(32, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX); // MatView and MatViewProj;
    rootParameters[1].InitAsConstants(16, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX); // MatWorld;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = ARRAYSIZE(rootParameters);
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    ID3DBlob* rootSignatureBlob = nullptr;
    D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, nullptr);
    device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));

    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    ID3D10Blob* vs = nullptr;
    ID3D10Blob* ps = nullptr;
    ID3D10Blob* errors = nullptr;

    D3DCompileFromFile(L"..\\Media\\StaticMesh.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", 0, 0, &vs, &errors);
    D3DCompileFromFile(L"..\\Media\\StaticMesh.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", 0, 0, &ps, &errors);

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.InputLayout.NumElements = ARRAYSIZE(inputLayout);
    psoDesc.InputLayout.pInputElementDescs = inputLayout;
    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    psoDesc.pRootSignature = mRootSignature.Get();

    psoDesc.VS.pShaderBytecode = vs->GetBufferPointer();
    psoDesc.VS.BytecodeLength = vs->GetBufferSize();
    psoDesc.PS.pShaderBytecode = ps->GetBufferPointer();
    psoDesc.PS.BytecodeLength = ps->GetBufferSize();
    
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = true;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.StencilEnable = false;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
   
    device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO));
    mPSO->SetName(L"ViewerPSO");
}

void MeshRenderer::RenderMeshes(ID3D12GraphicsCommandList* commandList, const std::vector<D3D12Mesh>& meshes, const Camera& camera)
{
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->SetPipelineState(mPSO.Get());
    commandList->SetGraphicsRootSignature(mRootSignature.Get());
    commandList->SetGraphicsRoot32BitConstants(0, 16, &camera.mMatView, 0);
    commandList->SetGraphicsRoot32BitConstants(0, 16, &camera.mMatViewProj, 16);

    for (const D3D12Mesh& mesh : meshes)
    {
        commandList->IASetIndexBuffer(&mesh.mIndexBuffer);
        commandList->IASetVertexBuffers(0, 1, &mesh.mVertexBuffer);
        commandList->SetGraphicsRoot32BitConstants(1, 16, &mesh.mMatWorld, 0);
        commandList->DrawIndexedInstanced(mesh.mNumIndices, 1, 0, 0, 0);
    }
}