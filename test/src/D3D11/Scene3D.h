/*
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#pragma once

#include <DXUT.h>
#include <strsafe.h>
#include <SDKmisc.h>
#include <SDKmesh.h>

#define MAX_PATH_STR 512

typedef enum
{
    SCENE_NO_GROUND_PLANE,
    SCENE_USE_GROUND_PLANE,
} SceneGroundPlaneType;

typedef enum
{
    SCENE_NO_SHADING,
    SCENE_USE_SHADING,
} SceneShadingType;

typedef enum
{
    SCENE_USE_ORBITAL_CAMERA,
    SCENE_USE_FIRST_PERSON_CAMERA,
} SceneCameraType;

struct MeshDescriptor
{
    LPWSTR Name;
    LPWSTR Path;
    SceneGroundPlaneType UseGroundPlane;
    SceneShadingType UseShading;
    SceneCameraType CameraType;
    float SceneScale;
};

struct SceneMesh
{
    HRESULT OnCreateDevice(ID3D11Device* pd3dDevice, MeshDescriptor &MeshDesc)
    {
        HRESULT hr = S_OK;
        WCHAR Filepath[MAX_PATH_STR+1];
        V_RETURN( DXUTFindDXSDKMediaFileCch(Filepath, MAX_PATH_STR, MeshDesc.Path) );
        V_RETURN( m_SDKMesh.Create(pd3dDevice, Filepath) );

        D3DXVECTOR3 bbExtents = m_SDKMesh.GetMeshBBoxExtents(0);
        D3DXVECTOR3 bbCenter  = m_SDKMesh.GetMeshBBoxCenter(0);
        m_GroundHeight = bbCenter.y - bbExtents.y;
        m_Desc = MeshDesc;
        return hr;
    }

    void OnDestroyDevice()
    {
        m_SDKMesh.Destroy();
    }

    CDXUTSDKMesh &GetSDKMesh()
    {
        return m_SDKMesh;
    }

    bool UseShading()
    {
        return (m_Desc.UseShading == SCENE_USE_SHADING);
    }

    bool UseGroundPlane()
    {
        return (m_Desc.UseGroundPlane == SCENE_USE_GROUND_PLANE);
    }

    bool UseOrbitalCamera()
    {
        return (m_Desc.CameraType == SCENE_USE_ORBITAL_CAMERA);
    }

    float GetSceneScale()
    {
        return m_Desc.SceneScale;
    }

    float GetGroundHeight()
    {
        return m_GroundHeight;
    }

protected:
    CDXUTSDKMesh    m_SDKMesh;
    float           m_GroundHeight;
    MeshDescriptor  m_Desc;
};

struct GlobalConstantBuffer
{
    D3DXMATRIX WorldView;
    D3DXMATRIX WorldViewInverse;
    D3DXMATRIX WorldViewProjection;
    float      IsWhite;
    float      Pad[3];
};

struct SceneViewInfo
{
    D3DXMATRIX WorldViewMatrix;
    D3DXMATRIX ProjectionMatrix;
    UINT StencilRef;
    BOOL UseGBufferNormals;
    BOOL AllowGroundPlane;
};

class SceneRenderer
{
public:
    SceneRenderer();

    HRESULT OnCreateDevice(ID3D11Device* pd3dDevice, bool ReversedDepthTest);
    void DumpGeometry(const SceneViewInfo* pSceneView, SceneMesh* pMesh);
    void RenderMesh(ID3D11DeviceContext* pD3DContext, const SceneViewInfo* pSceneView, SceneMesh* pMesh);
    void CopyColors(ID3D11DeviceContext* pD3DContext, ID3D11ShaderResourceView* pColorSRV);
    void OnDestroyDevice();

private:
    ID3D11InputLayout*          m_pInputLayout;
    ID3D11Buffer*               m_VertexBuffer;
    ID3D11Buffer*               m_IndexBuffer;

    GlobalConstantBuffer        m_Constants;
    ID3D11Buffer*               m_pConstantBuffer;

    ID3D11BlendState*           m_pBlendState_Disabled;
    ID3D11RasterizerState*      m_pRasterizerState_NoCull_NoScissor;
    ID3D11DepthStencilState*    m_pDepthStencilState_Disabled;
    ID3D11DepthStencilState*    m_pDepthStencilState_Enabled;
    ID3D11SamplerState*         m_pSamplerState_PointClamp;

    ID3D11VertexShader*         m_pFullScreenTriangleVS;
    ID3D11PixelShader*          m_pCopyColorPS;
    ID3D11VertexShader*         m_pGeometryVS;
    ID3D11PixelShader*          m_pGeometryColorNormalPS;
    ID3D11PixelShader*          m_pGeometryColorPS;
};
