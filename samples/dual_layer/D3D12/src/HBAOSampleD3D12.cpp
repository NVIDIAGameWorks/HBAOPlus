/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "HBAOSampleD3D12.h"
#include "DeviceManager12.h"
#include "d3dx12.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "GPUTimers.h"

static const uint32_t kNumAppDescriptors = 2;
static const uint32_t kNumImguiDescriptors = 2;
static const uint32_t kNumTotalAppDescriptors = kNumImguiDescriptors + kNumAppDescriptors;

GPUTimers gGPUTimers;

void HBAOSampleD3D12::Initialize()
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
    deviceParams.windowPosX = 0;
    deviceParams.windowPosY = 0;

    mDeviceManager.CreateWindowDeviceAndSwapChain(deviceParams, L"HBAO+ D3D12 Sample");
    mDeviceManager.SetPrimaryRenderTargetClearColor(false, nullptr);

    mDeviceManager.AddControllerToBack(this);

    mDevice = mDeviceManager.GetDevice();

    InitializeHBAOPlus();
    InitializeRenderTargets();

    mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator));

    mCommandQueue = mDeviceManager.GetDefaultQueue();
    mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&mCommandList));

    mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));

    ImGui_ImplDX12_Init(mDeviceManager.GetHWND(), mDevice, mCommandQueue, mCommandList.Get(), mSSAODescriptorHeapCBVSRVUAV.Get(), kNumAppDescriptors);

    gGPUTimers.Create(mDevice, mCommandQueue, NUM_GPU_TIMES);
    mScene.InitializeDefault();
    mSceneRenderer.InitializeWithScene(mDevice, mScene);
}

void HBAOSampleD3D12::Render(RenderTargetView RTV)
{
    gGPUTimers.BeginFrame(mCommandList.Get());
    D3D12_VIEWPORT viewport{};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)mWindowWidth;
    viewport.Height = (float)mWindowHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissorRect{};
    scissorRect.left = 0;
    scissorRect.right = mWindowWidth;
    scissorRect.top = 0;
    scissorRect.bottom = mWindowHeight;

    mCommandList->RSSetScissorRects(1, &scissorRect);
    mCommandList->RSSetViewports(1, &viewport);
    float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    mCommandList->ClearRenderTargetView(RTV, color, 0, nullptr);
    mCommandList->ClearDepthStencilView(mDepthStencilView[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    mCommandList->ClearDepthStencilView(mDepthStencilView[1], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 1. Render background meshes to mDepthStencilView[0]
    mCommandList->OMSetRenderTargets(1, &RTV, TRUE, &mDepthStencilView[0]);
    mSceneRenderer.RenderLayer(mCommandList.Get(), 0, mCamera);

    // 2. Copy mDepthStencilView[0] to mDepthStencilView[1]
    if (mAOParameters.EnableDualLayerAO)
    {
        mCommandList->CopyResource(mDepthStencil[1].Get(), mDepthStencil[0].Get());
    }

    // 3. Render foreground meshes to mDepthStencilView[0]
    mSceneRenderer.RenderLayer(mCommandList.Get(), 1, mCamera);

    GFSDK_SSAO_InputData_D3D12 InputData = {};
    InputData.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;

    if (mAOParameters.EnableDualLayerAO)
    {
        CD3DX12_GPU_DESCRIPTOR_HANDLE DepthSrvGpuHandle(mSSAODescriptorHeapCBVSRVUAV->GetGPUDescriptorHandleForHeapStart());
        InputData.DepthData.FullResDepthTextureSRV.pResource = mDepthStencil[0].Get();
        InputData.DepthData.FullResDepthTextureSRV.GpuHandle = DepthSrvGpuHandle.ptr;

        DepthSrvGpuHandle.Offset(1, mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
        InputData.DepthData.FullResDepthTexture2ndLayerSRV.pResource = mDepthStencil[1].Get();
        InputData.DepthData.FullResDepthTexture2ndLayerSRV.GpuHandle = DepthSrvGpuHandle.ptr;
    }
    else
    {
        CD3DX12_GPU_DESCRIPTOR_HANDLE DepthSrvGpuHandle(mSSAODescriptorHeapCBVSRVUAV->GetGPUDescriptorHandleForHeapStart());
        InputData.DepthData.FullResDepthTextureSRV.pResource = mDepthStencil[0].Get();
        InputData.DepthData.FullResDepthTextureSRV.GpuHandle = DepthSrvGpuHandle.ptr;
    }

    // DepthData
    InputData.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&mCamera.mMatProj);
    InputData.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
    InputData.DepthData.MetersToViewSpaceUnits = 1.0f;
    InputData.NormalData.Enable = false;
    
    GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_AO;

    // Set SSAO descriptor heap
    {
        ID3D12DescriptorHeap* descHeaps[] = { mSSAODescriptorHeapCBVSRVUAV.Get() };
        mCommandList->SetDescriptorHeaps(ARRAYSIZE(descHeaps), descHeaps);
    }

    GFSDK_SSAO_Output_D3D12 Output;
    GFSDK_SSAO_RenderTargetView_D3D12 rtv{};
    rtv.pResource = mDeviceManager.GetCurrentBackBuffer();
    rtv.CpuHandle = RTV.ptr;

    Output.pRenderTargetView = &rtv;
    
    {
        GPUTimer timer(&gGPUTimers, mCommandList.Get(), GPU_TIME_AO);
        GFSDK_SSAO_Status status = mSSAOContext->RenderAO(mCommandQueue, mCommandList.Get(), InputData, mAOParameters, Output, RenderMask);
        assert(status == GFSDK_SSAO_OK);
    }
    ImGui_ImplDX12_NewFrame();
    RenderUI();
    gGPUTimers.EndFrame(mCommandList.Get());
    mCommandList->Close();


    ID3D12CommandList* commandLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(1, commandLists);
    mCommandQueue->Signal(mFence.Get(), ++mFenceValue);

    while (mFence->GetCompletedValue() != mFenceValue)
    {
        __nop();
    }

    mFrameTimeMs = gGPUTimers.GetGPUTimeInMS(GPU_TIME_AO);
    mCommandAllocator->Reset();
    mCommandList->Reset(mCommandAllocator.Get(), nullptr);
}

void HBAOSampleD3D12::BackBufferResized(uint32_t width, uint32_t height, uint32_t sampleCount)
{
    mWindowWidth = width;
    mWindowHeight = height;

    mCamera.InitProjectionMatrix(width, height);

    InitializeRenderTargets();
}

bool HBAOSampleD3D12::InitializeHBAOPlus()
{
    InitializeHBAOParameters();

    const UINT NodeMask = 1;

    GFSDK_SSAO_CustomHeap CustomHeap;
    CustomHeap.new_ = ::operator new;
    CustomHeap.delete_ = ::operator delete;

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
    descriptorHeapDesc.NumDescriptors = kNumTotalAppDescriptors + GFSDK_SSAO_NUM_DESCRIPTORS_CBV_SRV_UAV_HEAP_D3D12;
    descriptorHeapDesc.NodeMask = NodeMask;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    mDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&mSSAODescriptorHeapCBVSRVUAV));

    SIZE_T t0 = mSSAODescriptorHeapCBVSRVUAV->GetCPUDescriptorHandleForHeapStart().ptr;

    descriptorHeapDesc.NumDescriptors = GFSDK_SSAO_NUM_DESCRIPTORS_RTV_HEAP_D3D12;
    descriptorHeapDesc.NodeMask = NodeMask;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    
    mDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&mSSAODescriptorHeapRTV));


    GFSDK_SSAO_DescriptorHeaps_D3D12 DescriptorHeaps;

    DescriptorHeaps.CBV_SRV_UAV.pDescHeap = mSSAODescriptorHeapCBVSRVUAV.Get();
    DescriptorHeaps.CBV_SRV_UAV.BaseIndex = kNumTotalAppDescriptors;

    DescriptorHeaps.RTV.pDescHeap = mSSAODescriptorHeapRTV.Get();
    DescriptorHeaps.RTV.BaseIndex = 0;

    GFSDK_SSAO_Status status = GFSDK_SSAO_CreateContext_D3D12(mDevice, NodeMask, DescriptorHeaps, &mSSAOContext, &CustomHeap);
    assert(status == GFSDK_SSAO_OK);
    return true;
}

bool HBAOSampleD3D12::InitializeRenderTargets()
{
    CD3DX12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0, mWindowWidth, mWindowHeight, 1, 1, DXGI_FORMAT_R32_TYPELESS, 1, 0, D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    for (uint32_t i = 0; i < 2; ++i)
    {
        mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0), IID_PPV_ARGS(&mDepthStencil[i]));
    }

    D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc{};
    dsvDescriptorHeapDesc.NumDescriptors = 2;
    dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvDescriptorHeapDesc.NodeMask = 0;
    dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    mDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&mDSVDescriptorHeap));

    for (uint32_t i = 0; i < 2; ++i)
    {
        mDepthStencilView[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), i, mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        mDevice->CreateDepthStencilView(mDepthStencil[i].Get(), &dsvDesc, mDepthStencilView[i]);

        D3D12_SHADER_RESOURCE_VIEW_DESC depthSRVDesc = {};

        depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
        depthSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        depthSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        depthSRVDesc.Texture2D.MipLevels = 1;
        depthSRVDesc.Texture2D.MostDetailedMip = 0; // No MIP
        depthSRVDesc.Texture2D.PlaneSlice = 0;
        depthSRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        mDepthSRV[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(mSSAODescriptorHeapCBVSRVUAV->GetCPUDescriptorHandleForHeapStart(), i, mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
        mDevice->CreateShaderResourceView(mDepthStencil[i].Get(), &depthSRVDesc, mDepthSRV[i]);
    }

    return true;
}

void HBAOSampleD3D12::Run()
{
    mDeviceManager.MessageLoop();
}

extern LRESULT ImGui_ImplDX12_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT HBAOSampleD3D12::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplDX12_WndProcHandler(hWnd, uMsg, wParam, lParam);
}
