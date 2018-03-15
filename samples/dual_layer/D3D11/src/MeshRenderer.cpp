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
#include <d3dcompiler.h>

#include "D3D11Mesh.h"
#include "Camera.h"

void MeshRenderer::Initialize(ID3D11Device* device)
{
    D3D11_INPUT_ELEMENT_DESC inputLayout[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    
    ID3D10Blob* vs = nullptr;
    ID3D10Blob* ps = nullptr;
    ID3D10Blob* errors = nullptr;

    D3DCompileFromFile(L"..\\Media\\StaticMesh.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vs, &errors);
    D3DCompileFromFile(L"..\\Media\\StaticMesh.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &ps, &errors);

    device->CreateInputLayout(inputLayout, ARRAYSIZE(inputLayout), vs->GetBufferPointer(), vs->GetBufferSize(), &mInputLayout);

    device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &mVertexShader);
    device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &mPixelShader);

    D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    device->CreateRasterizerState(&rasterizerDesc, &mRasterizerState);

    D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC(D3D11_DEFAULT);
    device->CreateBlendState(&blendDesc, &mBlendState);

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    device->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState);   

    D3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(sizeof(XMMATRIX) * 2, D3D11_BIND_CONSTANT_BUFFER);
    device->CreateBuffer(&bufferDesc, nullptr, &mPerFrameCB);

    bufferDesc.ByteWidth = sizeof(XMMATRIX);
    device->CreateBuffer(&bufferDesc, nullptr, &mPerObjectCB);
}

void MeshRenderer::RenderMeshes(ID3D11DeviceContext* deviceContext, const std::vector<D3D11Mesh>& meshes, const Camera& camera)
{
    deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(mInputLayout.Get());
    deviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);
    deviceContext->RSSetState(mRasterizerState.Get());
    float kBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    deviceContext->OMSetBlendState(mBlendState.Get(), kBlendFactor, 0xFFFFFFFF);
    deviceContext->OMSetDepthStencilState(mDepthStencilState.Get(), 0);
    ID3D11Buffer* constantBuffers[2] = { mPerFrameCB.Get(), mPerObjectCB.Get() };
    deviceContext->VSSetConstantBuffers(0, 2, constantBuffers);
    ID3D11Buffer* vertexBuffer = nullptr;
    UINT stride = sizeof(Mesh::Vertex);
    UINT offset = 0;

    deviceContext->UpdateSubresource(mPerFrameCB.Get(), 0, nullptr, camera.PerFrameData, 0, 0);

    for (const D3D11Mesh& mesh : meshes)
    {
        deviceContext->UpdateSubresource(mPerObjectCB.Get(), 0, nullptr, &mesh.mMatWorld, 0, 0);
        deviceContext->IASetIndexBuffer(mesh.mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        vertexBuffer = mesh.mVertexBuffer.Get();
        deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
        deviceContext->DrawIndexedInstanced(mesh.mNumIndices, 1, 0, 0, 0);
    }
}