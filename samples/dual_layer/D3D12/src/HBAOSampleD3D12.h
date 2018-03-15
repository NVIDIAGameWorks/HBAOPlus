/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include <HBAOSample.h>
#include "DeviceManager12.h"
#include <D3D12.h>
#include <wrl.h>
#include "SceneRenderer.h"

#include <GFSDK_SSAO.h>

using namespace Microsoft::WRL;

class HBAOSampleD3D12 : public HBAOSample, public IVisualController
{
public:
    void Initialize() override;
    void Run() override;

    void Render(RenderTargetView RTV) override;
    void BackBufferResized(uint32_t width, uint32_t height, uint32_t sampleCount) override;

private:
    bool InitializeHBAOPlus();
    bool InitializeRenderTargets();
    LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    DeviceManager                       mDeviceManager;
    ComPtr<ID3D12CommandAllocator>      mCommandAllocator;
    ComPtr<ID3D12GraphicsCommandList>   mCommandList;
    ComPtr<ID3D12Resource>              mDepthStencil[2];
    ComPtr<ID3D12Resource>              mNormalsGBuffer;
    D3D12_CPU_DESCRIPTOR_HANDLE         mDepthStencilView[2];    
    D3D12_CPU_DESCRIPTOR_HANDLE         mDepthSRV[2];
    ComPtr<ID3D12DescriptorHeap>        mDSVDescriptorHeap;
    ID3D12Device*                       mDevice = nullptr;

    ID3D12CommandQueue*                 mCommandQueue;
    ComPtr<ID3D12Fence>                 mFence;
    uint64_t                            mFenceValue = 0;

    SceneRenderer                       mSceneRenderer;

    ComPtr<ID3D12DescriptorHeap>        mSSAODescriptorHeapCBVSRVUAV;
    ComPtr<ID3D12DescriptorHeap>        mSSAODescriptorHeapRTV;
    GFSDK_SSAO_InputData_D3D12          mSSAOInputData;
    GFSDK_SSAO_Context_D3D12*           mSSAOContext;
};