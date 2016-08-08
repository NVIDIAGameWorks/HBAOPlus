/*
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "Scene3D.h"
#include "Shaders/bin/GeometryVS.h"
#include "Shaders/bin/GeometryColorNormalPS.h"
#include "Shaders/bin/GeometryColorPS.h"
#include "Shaders/bin/CopyColorPS.h"
#include "Shaders/bin/FullScreenTriangleVS.h"

struct Scene3DVertex
{
    D3DXVECTOR3 pos;
    D3DXVECTOR3 nor;
};

D3D11_INPUT_ELEMENT_DESC VertexLayout[] =
{
    { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

//-----------------------------------------------------------------------------
SceneRenderer::SceneRenderer()
    : m_pInputLayout(NULL)
    , m_VertexBuffer(NULL)
    , m_pConstantBuffer(NULL)
{
}

//-----------------------------------------------------------------------------
HRESULT SceneRenderer::OnCreateDevice(ID3D11Device* pd3dDevice, bool ReversedDepthTest)
{
    HRESULT hr = S_OK;

    // Create the input layout
    UINT NumElements = sizeof(VertexLayout)/sizeof(VertexLayout[0]);
    V( pd3dDevice->CreateInputLayout(VertexLayout, NumElements, g_GeometryVS, sizeof(g_GeometryVS), &m_pInputLayout) );

    // Create the floor plane
    float w = 1.0f;
    Scene3DVertex vertices[] =
    {
        { D3DXVECTOR3(-w, 0,  w), D3DXVECTOR3(0.0f, 1.0f, 0.0f) },
        { D3DXVECTOR3( w, 0,  w), D3DXVECTOR3(0.0f, 1.0f, 0.0f) },
        { D3DXVECTOR3( w, 0, -w), D3DXVECTOR3(0.0f, 1.0f, 0.0f) },
        { D3DXVECTOR3(-w, 0, -w), D3DXVECTOR3(0.0f, 1.0f, 0.0f) },
    };

    D3D11_BUFFER_DESC bd;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Scene3DVertex) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;
    V( pd3dDevice->CreateBuffer(&bd, &InitData, &m_VertexBuffer) );

    // Create index buffer
    DWORD indices[] =
    {
        0,1,2,
        0,2,3,
    };

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(DWORD) * 6;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;
    InitData.pSysMem = indices;
    V( pd3dDevice->CreateBuffer(&bd, &InitData, &m_IndexBuffer) );
    
    // Create the constant buffer
    static D3D11_BUFFER_DESC desc = 
    {
         sizeof(m_Constants), //ByteWidth
         D3D11_USAGE_DEFAULT, //Usage
         D3D11_BIND_CONSTANT_BUFFER, //BindFlags
         0, //CPUAccessFlags
         0  //MiscFlags
    };
    V( pd3dDevice->CreateBuffer(&desc, NULL, &m_pConstantBuffer) );

    D3D11_BLEND_DESC BlendStateDesc;
    BlendStateDesc.AlphaToCoverageEnable = FALSE;
    BlendStateDesc.IndependentBlendEnable = TRUE;
    for (int i = 0; i < 8; ++i)
    {
        BlendStateDesc.RenderTarget[i].BlendEnable = FALSE;
        BlendStateDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }
    V( pd3dDevice->CreateBlendState(&BlendStateDesc, &m_pBlendState_Disabled) );

    static D3D11_RASTERIZER_DESC RasterStateDesc = 
    {D3D11_FILL_SOLID, //FillMode
     D3D11_CULL_NONE, //CullMode
     0x0, //FrontCounterClockwise
     0x0/*0.000000f*/, //DepthBias
                   0.f, //DepthBiasClamp
                   0.f, //SlopeScaledDepthBias
     0x1, //DepthClipEnable
     0x0, //ScissorEnable
     0x0, //MultisampleEnable
     0x0  //AntialiasedLineEnable
    };
    V( pd3dDevice->CreateRasterizerState(&RasterStateDesc, &m_pRasterizerState_NoCull_NoScissor) );

    static D3D11_DEPTH_STENCIL_DESC DepthStencilStateDesc = 
    {FALSE, //DepthEnable
     D3D11_DEPTH_WRITE_MASK_ZERO, //DepthWriteMask
     D3D11_COMPARISON_NEVER, //DepthFunc
     FALSE, //StencilEnable
     0, //StencilReadMask
     0xFF, //StencilWriteMask
     
    {D3D11_STENCIL_OP_REPLACE, //StencilFailOp
     D3D11_STENCIL_OP_REPLACE, //StencilDepthFailOp
     D3D11_STENCIL_OP_REPLACE, //StencilPassOp
     D3D11_COMPARISON_ALWAYS  //StencilFunc
    }, //FrontFace
     
    {D3D11_STENCIL_OP_REPLACE, //StencilFailOp
     D3D11_STENCIL_OP_REPLACE, //StencilDepthFailOp
     D3D11_STENCIL_OP_REPLACE, //StencilPassOp
     D3D11_COMPARISON_ALWAYS  //StencilFunc
    }  //BackFace
    };
    V( pd3dDevice->CreateDepthStencilState(&DepthStencilStateDesc, &m_pDepthStencilState_Disabled) );

    DepthStencilStateDesc.DepthEnable = TRUE;
    DepthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    DepthStencilStateDesc.DepthFunc = ReversedDepthTest ? D3D11_COMPARISON_GREATER_EQUAL : D3D11_COMPARISON_LESS_EQUAL;
    DepthStencilStateDesc.StencilEnable = TRUE;
    V( pd3dDevice->CreateDepthStencilState(&DepthStencilStateDesc, &m_pDepthStencilState_Enabled) );

    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = 0.f;
    samplerDesc.BorderColor[1] = 0.f;
    samplerDesc.BorderColor[2] = 0.f;
    samplerDesc.BorderColor[3] = 0.f;
    samplerDesc.MinLOD = -D3D11_FLOAT32_MAX;
    samplerDesc.MaxLOD =  D3D11_FLOAT32_MAX;
    V( pd3dDevice->CreateSamplerState(&samplerDesc, &m_pSamplerState_PointClamp) );

    V( pd3dDevice->CreateVertexShader(g_FullScreenTriangleVS, sizeof(g_FullScreenTriangleVS), NULL, &m_pFullScreenTriangleVS) );
    V( pd3dDevice->CreatePixelShader(g_CopyColorPS, sizeof(g_CopyColorPS), NULL, &m_pCopyColorPS) );

    V( pd3dDevice->CreateVertexShader(g_GeometryVS, sizeof(g_GeometryVS), NULL, &m_pGeometryVS) );
    V( pd3dDevice->CreatePixelShader(g_GeometryColorNormalPS, sizeof(g_GeometryColorNormalPS), NULL, &m_pGeometryColorNormalPS) );
    V( pd3dDevice->CreatePixelShader(g_GeometryColorPS, sizeof(g_GeometryColorPS), NULL, &m_pGeometryColorPS) );

    return S_OK;
}

//-----------------------------------------------------------------------------
#if DUMP_GEOMETRY
void SceneRenderer::DumpGeometry(const SceneViewInfo* pSceneView, SceneMesh* pMesh)
{
    CDXUTSDKMesh &SDKMesh = pMesh->GetSDKMesh();

    for (UINT subset = 0; subset < SDKMesh.GetNumSubsets(0); ++subset)
    {
        SDKMESH_SUBSET* pSubset = SDKMesh.GetSubset(0, subset);

        FILE *fp = fopen("SibenikVertices.bin", "wb");
        if (fp)
        {
            BYTE* pVertexData = SDKMesh.GetRawVerticesAt(0);
            UINT Stride = (UINT)SDKMesh.GetVertexStride(0,0);
            UINT64 NVerts = SDKMesh.GetNumVertices(0,0);
            D3DXMATRIX WVMatrix = pSceneView->WorldViewMatrix;
            for (UINT64 i = 0; i < NVerts; ++i)
            {
                D3DXVECTOR3 *v = (D3DXVECTOR3 *)(&pVertexData[i * Stride]);
                D3DXVECTOR3 vCam;
                D3DXVec3TransformCoord(&vCam, v, &WVMatrix);

                // Write view-space vertex position for left-handed coord systems
                fwrite(&vCam, sizeof(vCam), 1, fp);
            }
            fclose(fp);
        }

        fp = fopen("SibenikIndices.bin", "wb");
        if (fp)
        {
            if (SDKMesh.GetIBFormat11(0) == DXGI_FORMAT_R32_UINT)
            {
                UINT* pIndexData = (UINT*)SDKMesh.GetRawIndicesAt(0);
                UINT64 NIndices = SDKMesh.GetNumIndices(0);
                fwrite(pIndexData, sizeof(UINT), NIndices, fp);
            }
            fclose(fp);
            exit(0);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
void SceneRenderer::RenderMesh(ID3D11DeviceContext* pD3DContext, const SceneViewInfo* pSceneView, SceneMesh* pMesh)
{
#if DUMP_GEOMETRY
    DumpGeometry(pSceneView, pMesh);
#endif

    pD3DContext->OMSetDepthStencilState(m_pDepthStencilState_Enabled, pSceneView->StencilRef);
    pD3DContext->OMSetBlendState(m_pBlendState_Disabled, NULL, 0xFFFFFFFF);
    pD3DContext->RSSetState(m_pRasterizerState_NoCull_NoScissor);

    ID3D11PixelShader* pGeometryPS = pSceneView->UseGBufferNormals ? m_pGeometryColorNormalPS : m_pGeometryColorPS;

    pD3DContext->VSSetShader(m_pGeometryVS, NULL, 0);
    pD3DContext->PSSetShader(pGeometryPS, NULL, 0);
    pD3DContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    pD3DContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

    // Update constant buffer
    m_Constants.WorldView = pSceneView->WorldViewMatrix;
    m_Constants.WorldViewProjection = m_Constants.WorldView * pSceneView->ProjectionMatrix;
    m_Constants.IsWhite = float(!pMesh->UseShading());
    D3DXMatrixInverse(&m_Constants.WorldViewInverse, NULL, &m_Constants.WorldView);
    pD3DContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &m_Constants, 0, 0);

    // Draw mesh
    CDXUTSDKMesh &SDKMesh = pMesh->GetSDKMesh();
    ID3D11Buffer* pVB = SDKMesh.GetVB11(0,0);
    ID3D11Buffer* pIB = SDKMesh.GetIB11(0);
    DXGI_FORMAT IBFormat = SDKMesh.GetIBFormat11(0);
    UINT Stride = (UINT)SDKMesh.GetVertexStride(0,0);
    UINT Offset = 0;
    pD3DContext->IASetVertexBuffers(0, 1, &pVB, &Stride, &Offset);
    pD3DContext->IASetIndexBuffer(pIB, IBFormat, 0);
    pD3DContext->IASetInputLayout(m_pInputLayout);
    pD3DContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (UINT subset = 0; subset < SDKMesh.GetNumSubsets(0); ++subset)
    {
        SDKMESH_SUBSET* pSubset = SDKMesh.GetSubset(0, subset);
        pD3DContext->DrawIndexed((UINT)pSubset->IndexCount, (UINT)pSubset->IndexStart, (UINT)pSubset->VertexStart);
    }

    // Draw ground plane
    if (pSceneView->AllowGroundPlane &&  pMesh->UseGroundPlane())
    {
        D3DXMATRIX mTranslate;
        D3DXMatrixTranslation(&mTranslate, 0.0f, pMesh->GetGroundHeight(), 0.0f);

        m_Constants.WorldView = mTranslate * m_Constants.WorldView;
        m_Constants.WorldViewProjection = m_Constants.WorldView * pSceneView->ProjectionMatrix;
        m_Constants.IsWhite = true;
        D3DXMatrixInverse(&m_Constants.WorldViewInverse, NULL, &m_Constants.WorldView);
        pD3DContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &m_Constants, 0, 0);

        UINT stride = sizeof(Scene3DVertex);
        UINT offset = 0;
        pD3DContext->IASetVertexBuffers    (0, 1, &m_VertexBuffer, &stride, &offset);
        pD3DContext->IASetIndexBuffer      (m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pD3DContext->IASetInputLayout      (m_pInputLayout);

        pD3DContext->DrawIndexed(6, 0, 0);
    }
}

//-----------------------------------------------------------------------------
void SceneRenderer::CopyColors(ID3D11DeviceContext* pD3DContext, ID3D11ShaderResourceView *pColorSRV)
{
    pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pD3DContext->OMSetDepthStencilState(m_pDepthStencilState_Disabled, 0);
    pD3DContext->OMSetBlendState(m_pBlendState_Disabled, NULL, 0xFFFFFFFF);
    pD3DContext->RSSetState(m_pRasterizerState_NoCull_NoScissor);

    pD3DContext->VSSetShader(m_pFullScreenTriangleVS, NULL, 0);
    pD3DContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

    pD3DContext->PSSetShader(m_pCopyColorPS, NULL, 0);
    pD3DContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    pD3DContext->PSSetSamplers(0, 1, &m_pSamplerState_PointClamp);
    pD3DContext->PSSetShaderResources(0, 1, &pColorSRV);

    pD3DContext->Draw(3, 0);
}

//-----------------------------------------------------------------------------
void SceneRenderer::OnDestroyDevice()
{
    SAFE_RELEASE(m_pInputLayout);
    SAFE_RELEASE(m_pConstantBuffer);
    SAFE_RELEASE(m_VertexBuffer);
    SAFE_RELEASE(m_IndexBuffer);
    SAFE_RELEASE(m_pBlendState_Disabled);
    SAFE_RELEASE(m_pRasterizerState_NoCull_NoScissor);
    SAFE_RELEASE(m_pDepthStencilState_Disabled);
    SAFE_RELEASE(m_pDepthStencilState_Enabled);
    SAFE_RELEASE(m_pSamplerState_PointClamp);
    SAFE_RELEASE(m_pFullScreenTriangleVS);
    SAFE_RELEASE(m_pCopyColorPS);
    SAFE_RELEASE(m_pGeometryVS);
    SAFE_RELEASE(m_pGeometryColorNormalPS);
    SAFE_RELEASE(m_pGeometryColorPS);
}
