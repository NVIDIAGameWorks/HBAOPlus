/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "HBAOSampleD3D11.h"
#include "DeviceManager11.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "GPUTimers.h"

static const uint32_t kNumAppDescriptors = 2;
static const uint32_t kNumImguiDescriptors = 2;
static const uint32_t kNumTotalAppDescriptors = kNumImguiDescriptors + kNumAppDescriptors;

GPUTimers gGPUTimers;

void HBAOSampleD3D11::Initialize()
{
    DeviceCreationParameters deviceParams{};
    deviceParams.backBufferWidth = mWindowWidth;
    deviceParams.backBufferHeight = mWindowHeight;
    deviceParams.driverType = D3D_DRIVER_TYPE_HARDWARE;
#if ENABLE_DEBUG_RUNTIME
    deviceParams.enableDebugRuntime = true;
#else
    deviceParams.enableDebugRuntime = false;
#endif
    deviceParams.featureLevel = D3D_FEATURE_LEVEL_11_0;
    deviceParams.refreshRate = 0;
    deviceParams.startFullscreen = false;
    deviceParams.startMaximized = false;
    deviceParams.swapChainBufferCount = 2;
    deviceParams.swapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    deviceParams.swapChainUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    mDeviceManager.CreateWindowDeviceAndSwapChain(deviceParams, L"HBAO+ D3D11 Sample");

    mDeviceManager.AddControllerToBack(this);

    mDevice = mDeviceManager.GetDevice();
    mDeviceContext = mDeviceManager.GetImmediateContext();

    InitializeHBAOPlus();
    InitializeRenderTargets();

    ImGui_ImplDX11_Init(mDeviceManager.GetHWND(), mDevice, mDeviceContext);
    ImGui_ImplDX11_CreateDeviceObjects();

    mScene.InitializeDefault();
    mSceneRenderer.InitializeWithScene(mDevice, mScene);

    gGPUTimers.Create(mDevice, NUM_GPU_TIMES);
}

void HBAOSampleD3D11::Render(RenderTargetView RTV)
{
    gGPUTimers.BeginFrame(mDeviceContext);
    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (FLOAT)mWindowWidth;
    viewport.Height = (FLOAT)mWindowHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D11_RECT scissorRect{};
    scissorRect.left = 0;
    scissorRect.right = mWindowWidth;
    scissorRect.top = 0;
    scissorRect.bottom = mWindowHeight;

    mDeviceContext->RSSetScissorRects(1, &scissorRect);
    mDeviceContext->RSSetViewports(1, &viewport);
    float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    mDeviceContext->ClearRenderTargetView(RTV, color);
    mDeviceContext->ClearDepthStencilView(mDepthStencilView[0].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    mDeviceContext->ClearDepthStencilView(mDepthStencilView[1].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // 1. Render background meshes to DepthStencilView[0]
    mDeviceContext->OMSetRenderTargets(1, &RTV, mDepthStencilView[0].Get());
    mSceneRenderer.RenderLayer(mDeviceContext, 0, mCamera); // draw the cube mesh (see Scene.cpp)

    // 2. Copy DepthStencilView[0] to DepthStencilView[1]
    if (mAOParameters.EnableDualLayerAO)
    {
        mDeviceContext->CopyResource(mDepthStencil[1].Get(), mDepthStencil[0].Get());
    }

    // 3. Render foreground meshes to DepthStencilView[0]
    mSceneRenderer.RenderLayer(mDeviceContext, 1, mCamera); // draw the actor mesh (see Scene.cpp)

    GFSDK_SSAO_InputData_D3D11 InputData = {};
    InputData.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;

    // FullResDepthTextureSRV
    if (mAOParameters.EnableDualLayerAO)
    {
        InputData.DepthData.pFullResDepthTextureSRV = mDepthSRV[0].Get();
        InputData.DepthData.pFullResDepthTexture2ndLayerSRV = mDepthSRV[1].Get();
    }
    else
    {
        InputData.DepthData.pFullResDepthTextureSRV = mDepthSRV[0].Get();
        InputData.DepthData.pFullResDepthTexture2ndLayerSRV = nullptr;
    }

    // DepthData
    InputData.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&mCamera.mMatProj);
    InputData.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
    InputData.DepthData.MetersToViewSpaceUnits = 1.0f;
    InputData.NormalData.Enable = false;

    GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_AO;


    GFSDK_SSAO_Output_D3D11 Output;

    Output.pRenderTargetView = RTV;
    
    {
        GPUTimer timer(&gGPUTimers, mDeviceContext, GPU_TIME_AO);
        GFSDK_SSAO_Status status = mSSAOContext->RenderAO(mDeviceContext, InputData, mAOParameters, Output, RenderMask);
        assert(status == GFSDK_SSAO_OK);
    }

    mFrameTimeMs = gGPUTimers.GetGPUTimeInMS(GPU_TIME_AO);


    ImGui_ImplDX11_NewFrame();
    RenderUI();
    gGPUTimers.EndFrame(mDeviceContext);
}

void HBAOSampleD3D11::BackBufferResized(uint32_t width, uint32_t height, uint32_t sampleCount)
{
    mWindowWidth = width;
    mWindowHeight = height;

    mCamera.InitProjectionMatrix(width, height);

    InitializeRenderTargets();
}

bool HBAOSampleD3D11::InitializeHBAOPlus()
{
    InitializeHBAOParameters();

    const UINT NodeMask = 1;

    GFSDK_SSAO_CustomHeap CustomHeap;
    CustomHeap.new_ = ::operator new;
    CustomHeap.delete_ = ::operator delete;

    GFSDK_SSAO_Status status = GFSDK_SSAO_CreateContext_D3D11(mDevice, &mSSAOContext, &CustomHeap);
    assert(status == GFSDK_SSAO_OK);
    return true;
}

bool HBAOSampleD3D11::InitializeRenderTargets()
{    
    D3D11_TEXTURE2D_DESC depthResourceDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32_TYPELESS, mWindowWidth, mWindowHeight, 1, 0, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
    for (uint32_t i = 0; i < 2; ++i)
    {
        mDevice->CreateTexture2D(&depthResourceDesc, nullptr, &mDepthStencil[i]);
    }

    for (uint32_t i = 0; i < 2; ++i)
    {        
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.Flags = 0;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        mDevice->CreateDepthStencilView(mDepthStencil[i].Get(), &dsvDesc, &mDepthStencilView[i]);

        D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc = {};

        depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;

        depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        depthSRVDesc.Texture2D.MipLevels = 1;
        depthSRVDesc.Texture2D.MostDetailedMip = 0; // No MIP
        mDevice->CreateShaderResourceView(mDepthStencil[i].Get(), &depthSRVDesc, &mDepthSRV[i]);
    }

    return true;
}

void HBAOSampleD3D11::Run()
{
    mDeviceManager.MessageLoop();
}

extern LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT HBAOSampleD3D11::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplDX11_WndProcHandler(hWnd, uMsg, wParam, lParam);
}
