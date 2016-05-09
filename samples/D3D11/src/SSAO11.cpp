/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#include <D3D11.h>
#include <DirectXMath.h>
#include <assert.h>
#include <stdexcept>
#include <vector>

#include "GFSDK_SSAO.h"
#include "AntTweakBar.h"
#include "DeviceManager.h"
#include "GPUTimers.h"
#include "BinMeshReader.h"
#include "SceneRTs.h"

#include "shaders/bin/GeometryVS.h"
#include "shaders/bin/GeometryPS.h"

#define VERTEX_BIN_FILE_PATH "..\\src\\SibenikVertices.bin"
#define INDEX_BIN_FILE_PATH "..\\src\\SibenikIndices.bin"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)         { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)          { if (p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(A)         (sizeof(A) / sizeof(A[0]))
#endif

#ifndef SAFE_D3D_CALL
#define SAFE_D3D_CALL(x) { if(FAILED(x)) assert(0); }
#endif

enum GPUTimeId
{
    GPU_TIME_Z,
    GPU_TIME_AO,
    GPU_TIME_UI,
    NUM_GPU_TIMES
};
GPUTimers g_GPUTimers;

DeviceManager* g_DeviceManager = NULL;
bool g_bRenderHUD = true;

using namespace DirectX;

class VisualController : public IVisualController
{
public:
    VisualController()
        : m_pAOContext(NULL)
        , m_AORadius(2.f)
        , m_AOBias(0.2f)
        , m_BlurAO(true)
        , m_BlurSharpness(32.f)
        , m_PowerExponent(2.f)
        , m_SmallScaleAO(1.f)
        , m_LargeScaleAO(1.f)
        , m_pVB(NULL)
        , m_pIB(NULL)
        , m_pConstantBuffer(NULL)
        , m_pInputLayout(NULL)
        , m_pGeometryVS(NULL)
        , m_pGeometryPS(NULL)
        , m_pDepthStencilState_Enabled(NULL)
        , m_pBlendState_Disabled(NULL)
        , m_pRasterizerState_NoCull_NoScissor(NULL)
        , m_pSceneRTs(NULL)
        , m_UseGBufferNormals(false)
        , m_DebugNormals(false)
    {
        m_BackBufferDesc = {};
    }

    virtual LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (uMsg == WM_KEYDOWN)
        {
            int iKeyPressed = static_cast<int>(wParam);

            switch (iKeyPressed)
            {
            case VK_TAB:
                g_bRenderHUD = !g_bRenderHUD;
                return 0;
                break;

            case VK_ESCAPE:
                PostQuitMessage(0);
                break;

            default:
                break;
            }
        }

        if (g_bRenderHUD || uMsg == WM_KEYDOWN || uMsg == WM_CHAR)
        {
            if (TwEventWin(hWnd, uMsg, wParam, lParam))
            {
                return 0; // Event has been handled by AntTweakBar
            }
        }

        return 1;
    }

    virtual void Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV)
    {
        g_GPUTimers.BeginFrame(pDeviceContext);

        DrawGeometry(pDevice, pDeviceContext);

        DrawHBAO(pDevice, pDeviceContext, pRTV);

        DrawUI(pDevice, pDeviceContext, pRTV, pDSV);

        g_GPUTimers.EndFrame(pDeviceContext);
    }

    static void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
    {
        destinationClientString = sourceLibraryString;
    }

    virtual HRESULT DeviceCreated(ID3D11Device* pDevice)
    {
        TwInit(TW_DIRECT3D11, pDevice);
        TwCopyStdStringToClientFunc(CopyStdStringToClient);
        InitGeometry(pDevice);
        InitHBAO(pDevice);
        InitUI();

        return S_OK;
    }

    virtual void DeviceDestroyed()
    {
        TwTerminate();
        ReleaseGeometry();
        ReleaseHBAO();
        SAFE_DELETE(m_pSceneRTs);
    }

    virtual void BackBufferResized(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
    {
        TwWindowSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
        ReallocateSceneRTs(pDevice, pBackBufferSurfaceDesc);
        m_BackBufferDesc = *pBackBufferSurfaceDesc;
    }

private:
    void InitGeometry(ID3D11Device* pDevice)
    {
        if (!LoadVertices(VERTEX_BIN_FILE_PATH, m_Mesh.vertices))
        {
            throw std::runtime_error("Failed to load " VERTEX_BIN_FILE_PATH);
        }
        if (!LoadIndices(INDEX_BIN_FILE_PATH, m_Mesh.indices))
        {
            throw std::runtime_error("Failed to load " INDEX_BIN_FILE_PATH);
        }

        SAFE_D3D_CALL(pDevice->CreateVertexShader(g_GeometryVS, sizeof(g_GeometryVS), NULL, &m_pGeometryVS));
        SAFE_D3D_CALL(pDevice->CreatePixelShader(g_GeometryPS, sizeof(g_GeometryPS), NULL, &m_pGeometryPS));
    }

    void ReallocateSceneRTs(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
    {
        SceneRTs::Desc desc;
        desc.OutputWidth = pBackBufferSurfaceDesc->Width;
        desc.OutputHeight = pBackBufferSurfaceDesc->Height;
        desc.BorderPixels = 0;
        desc.SampleCount = 1;

        SAFE_DELETE(m_pSceneRTs);
        m_pSceneRTs = new SceneRTs(pd3dDevice, desc);
    }

    void InitHBAO(ID3D11Device* pDevice)
    {
        GFSDK_SSAO_CustomHeap CustomHeap;
        CustomHeap.new_ = ::operator new;
        CustomHeap.delete_ = ::operator delete;

        GFSDK_SSAO_Status status;
        status = GFSDK_SSAO_CreateContext_D3D11(pDevice, &m_pAOContext, &CustomHeap);
        assert(status == GFSDK_SSAO_OK);

        g_GPUTimers.Create(pDevice, NUM_GPU_TIMES);
    }

    void InitUI()
    {
        TwBar* bar = TwNewBar("barMain");

        TwAddVarRW(bar, "Radius", TW_TYPE_FLOAT, &m_AORadius, "min=1.0 max=8.0 group=AO");
        TwAddVarRW(bar, "Bias", TW_TYPE_FLOAT, &m_AOBias, "min=0.0 max=0.5 group=AO");
        TwAddVarRW(bar, "SmallScaleAO", TW_TYPE_FLOAT, &m_SmallScaleAO, "min=0.0 max=2.0 group=AO");
        TwAddVarRW(bar, "LargeScaleAO", TW_TYPE_FLOAT, &m_LargeScaleAO, "min=0.0 max=2.0 group=AO");
        TwAddVarRW(bar, "Power Exponent", TW_TYPE_FLOAT, &m_PowerExponent, "min=0.0 max=4.0 group=AO");
        TwAddVarRW(bar, "Enable Blur", TW_TYPE_BOOLCPP, &m_BlurAO, "group=Blur");
        TwAddVarRW(bar, "Blur Sharpness", TW_TYPE_FLOAT, &m_BlurSharpness, "group=Blur min=0.0 max=100.0");
        TwAddVarRW(bar, "GBuffer Normals", TW_TYPE_BOOLCPP, &m_UseGBufferNormals, "group=Normals");
        TwAddVarRW(bar, "Debug Normals", TW_TYPE_BOOLCPP, &m_DebugNormals, "group=Normals");
    }

    void UpdateUI()
    {
        const int barWidth = 200;
        const int barHeight = 220;
        const int border = 20;

        char buffer[2048];
        _snprintf_s(buffer, sizeof(buffer),
            "barMain label='HBAO+' color='19 25 19' alpha=128 size='%d %d' position='%d %d' valueswidth=fit",
            barWidth, barHeight,
            m_BackBufferDesc.Width - barWidth - border, border);
        TwDefine(buffer);
    }

    void DrawText()
    {
        char msg[1024];

        TwBeginText(2, 0, 0, 220);

        GFSDK_SSAO_Version Version;
        GFSDK_SSAO_Status Status;
        Status = GFSDK_SSAO_GetVersion(&Version);
        assert(Status == GFSDK_SSAO_OK);

        unsigned int Color = 0xFFFFFFFF;
        unsigned int BgColor = 0xFF000000;

        _snprintf_s(msg, sizeof(msg), "D3D11 HBAO+ %d.%d.%d.%d", Version.Major, Version.Minor, Version.Branch, Version.Revision);
        TwAddTextLine(msg, Color, BgColor);

        _snprintf_s(msg, sizeof(msg), "Resolution: %d x %d", m_BackBufferDesc.Width, m_BackBufferDesc.Height);
        TwAddTextLine(msg, Color, BgColor);

        _snprintf_s(msg, sizeof(msg), "GPU times (ms): Z=%.2f AO=%.2f",
            g_GPUTimers.GetGPUTimeInMS(GPU_TIME_Z),
            g_GPUTimers.GetGPUTimeInMS(GPU_TIME_AO));
        TwAddTextLine(msg, Color, BgColor);

        _snprintf_s(msg, sizeof(msg), "Allocated Video Memory: %d MB", m_pAOContext->GetAllocatedVideoMemoryBytes() / (1024 * 1024));
        TwAddTextLine(msg, Color, BgColor);

        TwEndText();
    }

    void DrawUI(ID3D11Device* , ID3D11DeviceContext* pDeviceContext, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV)
    {
        GPUTimer TimerScope(&g_GPUTimers, pDeviceContext, GPU_TIME_UI);

        if (!g_bRenderHUD)
        {
            return;
        }

        pDeviceContext->OMSetRenderTargets(1, &pRTV, pDSV);
        pDeviceContext->OMSetDepthStencilState(NULL, 0);
        pDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);

        UpdateUI();
        DrawText();
        TwDraw();
    }

    void DrawGeometry(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
    {
        GPUTimer TimerScope(&g_GPUTimers, pDeviceContext, GPU_TIME_Z);

        D3D11_VIEWPORT Viewport;
        Viewport.TopLeftX = 0.f;
        Viewport.TopLeftY = 0.f;
        Viewport.MinDepth = 0.f;
        Viewport.MaxDepth = 1.f;
        Viewport.Width = FLOAT(m_pSceneRTs->Width);
        Viewport.Height = FLOAT(m_pSceneRTs->Height);
        pDeviceContext->RSSetViewports(1, &Viewport);

        float ClearColor[4] = { 1.f, 1.f, 1.f, 0.f };
        pDeviceContext->ClearRenderTargetView(m_pSceneRTs->ColorRTV, ClearColor);
        pDeviceContext->ClearDepthStencilView(m_pSceneRTs->DepthStencilDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

        if (m_UseGBufferNormals)
        {
            ID3D11RenderTargetView* pMRTs[] = { NULL, m_pSceneRTs->NormalRTV };
            pDeviceContext->OMSetRenderTargets(SIZEOF_ARRAY(pMRTs), pMRTs, m_pSceneRTs->DepthStencilDSV);
        }
        else
        {
            pDeviceContext->OMSetRenderTargets(0, NULL, m_pSceneRTs->DepthStencilDSV);
        }

        if (!m_pDepthStencilState_Enabled)
        {
            static D3D11_DEPTH_STENCIL_DESC DepthStencilStateDesc =
            {
                FALSE, //DepthEnable
                D3D11_DEPTH_WRITE_MASK_ZERO, //DepthWriteMask
                D3D11_COMPARISON_NEVER, //DepthFunc
                FALSE, //StencilEnable
                0, //StencilReadMask
                0xFF, //StencilWriteMask

                { D3D11_STENCIL_OP_REPLACE, //StencilFailOp
                D3D11_STENCIL_OP_REPLACE, //StencilDepthFailOp
                D3D11_STENCIL_OP_REPLACE, //StencilPassOp
                D3D11_COMPARISON_ALWAYS  //StencilFunc
                }, //FrontFace

                { D3D11_STENCIL_OP_REPLACE, //StencilFailOp
                D3D11_STENCIL_OP_REPLACE, //StencilDepthFailOp
                D3D11_STENCIL_OP_REPLACE, //StencilPassOp
                D3D11_COMPARISON_ALWAYS  //StencilFunc
                }  //BackFace
            };
            DepthStencilStateDesc.DepthEnable = TRUE;
            DepthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            DepthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
            DepthStencilStateDesc.StencilEnable = FALSE;
            SAFE_D3D_CALL(pDevice->CreateDepthStencilState(&DepthStencilStateDesc, &m_pDepthStencilState_Enabled));
        }
        pDeviceContext->OMSetDepthStencilState(m_pDepthStencilState_Enabled, 0);

        if (!m_pBlendState_Disabled)
        {
            D3D11_BLEND_DESC BlendStateDesc;
            BlendStateDesc.AlphaToCoverageEnable = FALSE;
            BlendStateDesc.IndependentBlendEnable = TRUE;
            for (int i = 0; i < SIZEOF_ARRAY(BlendStateDesc.RenderTarget); ++i)
            {
                BlendStateDesc.RenderTarget[i].BlendEnable = FALSE;
                BlendStateDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            }
            SAFE_D3D_CALL(pDevice->CreateBlendState(&BlendStateDesc, &m_pBlendState_Disabled));
        }
        pDeviceContext->OMSetBlendState(m_pBlendState_Disabled, NULL, 0xFFFFFFFF);

        if (!m_pRasterizerState_NoCull_NoScissor)
        {
            static D3D11_RASTERIZER_DESC RasterStateDesc =
            {
                D3D11_FILL_SOLID, //FillMode
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
            SAFE_D3D_CALL(pDevice->CreateRasterizerState(&RasterStateDesc, &m_pRasterizerState_NoCull_NoScissor));
        }
        pDeviceContext->RSSetState(m_pRasterizerState_NoCull_NoScissor);

        pDeviceContext->VSSetShader(m_pGeometryVS, NULL, 0);
        pDeviceContext->PSSetShader(m_pGeometryPS, NULL, 0);

        struct GlobalConstantBuffer
        {
            XMMATRIX WorldView;
            XMMATRIX WorldViewInverse;
            XMMATRIX WorldViewProjection;
            float   IsWhite;
            float   Pad[3];
        };

        if (!m_pConstantBuffer)
        {
            static D3D11_BUFFER_DESC desc =
            {
                sizeof(GlobalConstantBuffer), //ByteWidth
                D3D11_USAGE_DEFAULT, //Usage
                D3D11_BIND_CONSTANT_BUFFER, //BindFlags
                0, //CPUAccessFlags
                0  //MiscFlags
            };
            SAFE_D3D_CALL(pDevice->CreateBuffer(&desc, NULL, &m_pConstantBuffer));
        }

        float NearPlane = .01f;
        float FarPlane = 500.0f;
        float AspectRatio = (float)m_BackBufferDesc.Width / (float)m_BackBufferDesc.Height;
        m_ViewInfo.ProjectionMatrix = XMMatrixPerspectiveFovLH(40.f * 3.141592f / 180.f, AspectRatio, NearPlane, FarPlane);
        m_ViewInfo.WorldViewMatrix = XMMatrixIdentity();

        XMMATRIX WorldViewMatrix;
        GlobalConstantBuffer Constants = {};
        Constants.WorldView = m_ViewInfo.WorldViewMatrix;
        Constants.WorldViewProjection = m_ViewInfo.ProjectionMatrix;
        Constants.IsWhite = true;
        Constants.WorldViewInverse = XMMatrixInverse(NULL, Constants.WorldView);
        pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &Constants, 0, 0);

        pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
        pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

        if (!m_pVB)
        {
            static D3D11_BUFFER_DESC BufferDesc =
            {
                UINT(m_Mesh.vertices.size() * sizeof(m_Mesh.vertices[0])), //ByteWidth
                D3D11_USAGE_DEFAULT, //Usage
                D3D11_BIND_VERTEX_BUFFER, //BindFlags
                0, //CPUAccessFlags
                0, //MiscFlags
                0  //StructureByteStride
            };

            D3D11_SUBRESOURCE_DATA SubResourceData = { m_Mesh.vertices.data() /*pSysMem*/, 0 /*SysMemPitch*/, 0 /*SysMemSlicePitch*/ };
            SAFE_D3D_CALL(pDevice->CreateBuffer(&BufferDesc, &SubResourceData, &m_pVB));
        }

        if (!m_pIB)
        {
            static D3D11_BUFFER_DESC BufferDesc =
            {
                UINT(m_Mesh.indices.size() * sizeof(m_Mesh.indices[0])), //ByteWidth
                D3D11_USAGE_DEFAULT, //Usage
                D3D11_BIND_INDEX_BUFFER, //BindFlags
                0, //CPUAccessFlags
                0, //MiscFlags
                0  //StructureByteStride
            };

            D3D11_SUBRESOURCE_DATA SubResourceData = { m_Mesh.indices.data() /*pSysMem*/, 0 /*SysMemPitch*/, 0 /*SysMemSlicePitch*/ };
            SAFE_D3D_CALL(pDevice->CreateBuffer(&BufferDesc, &SubResourceData, &m_pIB));
        }

        if (!m_pInputLayout)
        {
            D3D11_INPUT_ELEMENT_DESC VertexLayout[] =
            {
                { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };

            UINT NumElements = sizeof(VertexLayout) / sizeof(VertexLayout[0]);
            SAFE_D3D_CALL(pDevice->CreateInputLayout(VertexLayout, NumElements, g_GeometryVS, sizeof(g_GeometryVS), &m_pInputLayout));
        }

        assert(sizeof(m_Mesh.indices[0]) == 4);
        DXGI_FORMAT IBFormat = DXGI_FORMAT_R32_UINT;
        UINT Stride = sizeof(m_Mesh.vertices[0]);
        UINT Offset = 0;
        pDeviceContext->IASetVertexBuffers(0, 1, &m_pVB, &Stride, &Offset);
        pDeviceContext->IASetIndexBuffer(m_pIB, IBFormat, 0);
        pDeviceContext->IASetInputLayout(m_pInputLayout);
        pDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        pDeviceContext->DrawIndexed(UINT(m_Mesh.indices.size()), 0, 0);
    }

    void DrawHBAO(ID3D11Device*, ID3D11DeviceContext* pDeviceContext, ID3D11RenderTargetView* pRTV)
    {
        GPUTimer TimerScope(&g_GPUTimers, pDeviceContext, GPU_TIME_AO);

        GFSDK_SSAO_InputData_D3D11 Input;

        Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
        Input.DepthData.pFullResDepthTextureSRV = m_pSceneRTs->DepthStencilSRV;
        Input.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((CONST FLOAT*)&m_ViewInfo.ProjectionMatrix);
        Input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
        Input.DepthData.MetersToViewSpaceUnits = 0.005f;

        if (m_UseGBufferNormals)
        {
            Input.NormalData.Enable = TRUE;
            Input.NormalData.pFullResNormalTextureSRV = m_pSceneRTs->NormalSRV;
            Input.NormalData.WorldToViewMatrix.Data = GFSDK_SSAO_Float4x4((CONST FLOAT*)&m_ViewInfo.WorldViewMatrix);
            Input.NormalData.WorldToViewMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
            Input.NormalData.DecodeScale = 2.f;
            Input.NormalData.DecodeBias = -1.f;
        }

        GFSDK_SSAO_Output_D3D11 Output;
        Output.pRenderTargetView = pRTV;// m_pSceneRTs->ColorRTV;
        Output.Blend.Mode = GFSDK_SSAO_OVERWRITE_RGB;

        GFSDK_SSAO_Parameters AOParams;
        AOParams.Radius = m_AORadius;
        AOParams.Bias = m_AOBias;
        AOParams.SmallScaleAO = m_SmallScaleAO;
        AOParams.LargeScaleAO = m_LargeScaleAO;
        AOParams.PowerExponent = m_PowerExponent;
        AOParams.Blur.Enable = m_BlurAO;
        AOParams.Blur.Sharpness = m_BlurSharpness;
        AOParams.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;

        const GFSDK_SSAO_RenderMask RenderMask = m_DebugNormals ? GFSDK_SSAO_RENDER_DEBUG_NORMAL_Z : GFSDK_SSAO_RENDER_AO;

        GFSDK_SSAO_Status Status;
        Status = m_pAOContext->RenderAO(pDeviceContext, Input, AOParams, Output, RenderMask);
        assert(Status == GFSDK_SSAO_OK);
    }

    void ReleaseHBAO()
    {
        SAFE_RELEASE(m_pAOContext);

        g_GPUTimers.Release();
    }

    void ReleaseGeometry()
    {
        SAFE_RELEASE(m_pVB);
        SAFE_RELEASE(m_pIB);
        SAFE_RELEASE(m_pConstantBuffer);
        SAFE_RELEASE(m_pInputLayout);

        SAFE_RELEASE(m_pDepthStencilState_Enabled);
        SAFE_RELEASE(m_pBlendState_Disabled);
        SAFE_RELEASE(m_pRasterizerState_NoCull_NoScissor);
    }

    DXGI_SURFACE_DESC m_BackBufferDesc;
    GFSDK_SSAO_Context_D3D11* m_pAOContext;
    float m_AORadius;
    float m_AOBias;
    bool m_BlurAO;
    float m_BlurSharpness;
    float m_PowerExponent;
    float m_SmallScaleAO;
    float m_LargeScaleAO;

    Mesh m_Mesh;
    ID3D11Buffer* m_pVB;
    ID3D11Buffer* m_pIB;
    ID3D11Buffer* m_pConstantBuffer;
    ID3D11InputLayout* m_pInputLayout;
    ID3D11VertexShader* m_pGeometryVS;
    ID3D11PixelShader* m_pGeometryPS;

    struct ViewInfo
    {
        XMMATRIX WorldViewMatrix;
        XMMATRIX ProjectionMatrix;
    };
    ViewInfo m_ViewInfo;

    ID3D11DepthStencilState* m_pDepthStencilState_Enabled;
    ID3D11BlendState*        m_pBlendState_Disabled;
    ID3D11RasterizerState*   m_pRasterizerState_NoCull_NoScissor;

    SceneRTs* m_pSceneRTs;
    bool m_UseGBufferNormals;
    bool m_DebugNormals;
};

int WINAPI wWinMain(HINSTANCE , HINSTANCE , LPWSTR , int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    g_DeviceManager = new DeviceManager();
    VisualController Controller;
    g_DeviceManager->AddControllerToFront(&Controller);

    DeviceCreationParameters DeviceParams;
#if _DEBUG
    DeviceParams.createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    DeviceParams.swapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DeviceParams.swapChainSampleCount = 1;
    DeviceParams.startFullscreen = false;
    DeviceParams.backBufferWidth = 1280;
    DeviceParams.backBufferHeight = 720;

    if(FAILED(g_DeviceManager->CreateWindowDeviceAndSwapChain(DeviceParams, L"NVIDIA HBAO+")))
    {
        MessageBox(NULL, L"Cannot initialize the D3D11 device with the requested parameters", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    g_DeviceManager->MessageLoop();

    delete g_DeviceManager;    // destructor calls Shutdown()

    return 0;
}
