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

#if SUPPORT_D3D12

#include <Windows.h>
#include <tchar.h>
#include <wrl/client.h>
#include <stdexcept>
#include <dxgi1_3.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include "d3dx12.h"

typedef D3D12_SHADER_BYTECODE GFSDK_D3D12_VertexShader;
typedef D3D12_SHADER_BYTECODE GFSDK_D3D12_GeometryShader;
typedef D3D12_SHADER_BYTECODE GFSDK_D3D12_PixelShader;

//--------------------------------------------------------------------------------
struct ShaderResourceView
{
    ShaderResourceView()
    {
        memset(this, 0, sizeof(*this));
    }
    ShaderResourceView(GFSDK_SSAO_ShaderResourceView_D3D12 A)
        : pResource(A.pResource)
        , GpuHandle({ A.GpuHandle })
    {
    }
    ID3D12Resource*                 pResource;
    D3D12_GPU_DESCRIPTOR_HANDLE     GpuHandle;
};

struct RenderTargetView
{
    RenderTargetView()
    {
        memset(this, 0, sizeof(*this));
    }
    RenderTargetView(GFSDK_SSAO_RenderTargetView_D3D12 A)
        : pResource(A.pResource)
        , CpuHandle({ A.CpuHandle })
    {
    }
    ID3D12Resource*                 pResource;
    D3D12_CPU_DESCRIPTOR_HANDLE     CpuHandle;
};

struct GFSDK_D3D12_DescriptorHeap
{
    D3D12_DESCRIPTOR_HEAP_DESC Desc;
    ID3D12DescriptorHeap* pDescHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE hCPUHeap;
    D3D12_GPU_DESCRIPTOR_HANDLE hGPUHeap;
    UINT HandleIncrementSize;
    UINT NumDescriptors;
    UINT BaseIndex;

    void InitDescriptorHeap(ID3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, const GFSDK_SSAO_DescriptorHeapRange_D3D12* pHeapInfo, bool bShaderVisible)
    {
        NumDescriptors = pHeapInfo->NumDescriptors;
        pDescHeap = pHeapInfo->pDescHeap;
        BaseIndex = pHeapInfo->BaseIndex;

        if (NumDescriptors > 0)
        {
            pDescHeap->AddRef();
            hCPUHeap = pDescHeap->GetCPUDescriptorHandleForHeapStart();
            if (bShaderVisible)
            {
                hGPUHeap = pDescHeap->GetGPUDescriptorHandleForHeapStart();
            }
            HandleIncrementSize = pDevice->GetDescriptorHandleIncrementSize(HeapType);
        }
        else
        {
            pDescHeap = nullptr;
            HandleIncrementSize = 0;
        }
    }

    void Release()
    {
        SAFE_RELEASE(pDescHeap);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT index)
    {
        ASSERT(index < NumDescriptors);
        return{ hCPUHeap.ptr + (BaseIndex + index) * HandleIncrementSize };
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT index)
    {
        ASSERT(index < NumDescriptors);
        return{ hGPUHeap.ptr + (BaseIndex + index) * HandleIncrementSize };
    }
};

enum CBVSRVUAVLayoutBase
{
    eGlobalCB = 0,
    ePerPassCB,
    ePerPassCBEnd = ePerPassCB + 16 - 1,
    eFullResViewDepthTexture,
    eFullResNormalTexture,
    eQuarterResViewDepthTextureArray,
    eQuarterResViewDepthTextureArrayEnd = eQuarterResViewDepthTextureArray + 16,
    eQuarterResAOTextureArray,
    eQuarterResAOTextureArrayEnd = eQuarterResAOTextureArray + 16,
    eFullResAOZTexture,
    eFullResAOZTexture2,
    eCBVSRVUAVLayoutBaseMax
};

enum RTVLayoutBase
{
    eFullResViewDepthTextureRTV,
    eFullResNormalTextureRTV,
    eQuarterResViewDepthTextureArrayRTV,
    eQuarterResAOTextureArrayRTV = eQuarterResViewDepthTextureArrayRTV + 1 + 16,
    eFullResAOZTextureRTV = eQuarterResAOTextureArrayRTV + 1 + 16,
    eFullResAOZTexture2RTV,
    eRTVLayoutBaseMax
};

struct GFSDK_D3D12_DescriptorHeaps
{
    GFSDK_D3D12_DescriptorHeap CBVSRVUAV;
    GFSDK_D3D12_DescriptorHeap RTV;

    void InitDescriptorHeaps(ID3D12Device* pDevice, const GFSDK_SSAO_DescriptorHeaps_D3D12& DescriptorHeaps)
    {
        CBVSRVUAV.InitDescriptorHeap(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, &DescriptorHeaps.CBV_SRV_UAV, true);
        RTV.InitDescriptorHeap(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, &DescriptorHeaps.RTV, false);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(CBVSRVUAVLayoutBase BaseOffset, UINT Index)
    {
        return CBVSRVUAV.GetGPUHandle(BaseOffset + Index);
    }

    void Release()
    {
        RTV.Release();
        CBVSRVUAV.Release();
    }
};

inline void GFSDK_D3D12_SetResourceBarrier(ID3D12GraphicsCommandList* commandList,
    ID3D12Resource* res,
    D3D12_RESOURCE_STATES before,
    D3D12_RESOURCE_STATES after)
{
    D3D12_RESOURCE_BARRIER desc = {};
    desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    desc.Transition.pResource = res;
    desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    desc.Transition.StateBefore = before;
    desc.Transition.StateAfter = after;
    desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    commandList->ResourceBarrier(1, &desc);
}

struct GFSDK_D3D12_GraphicsContext
{
    ID3D12Device* pDevice;
    GFSDK_D3D12_DescriptorHeaps DescHeaps;
    ID3D12GraphicsCommandList* pCmdList;
    ID3D12CommandQueue* pCmdQueue;
    ID3D12Fence* pFence;
    HANDLE hFenceEvent = 0;
    UINT64 FenceValue;
    UINT NodeMask;

    void Init(ID3D12Device* _pDevice, const GFSDK_SSAO_DescriptorHeaps_D3D12& DescriptorHeaps, UINT _NodeMask)
    {
        ASSERT(pDevice == NULL);
        ASSERT(pCmdList == NULL);
        ASSERT(pCmdQueue == NULL);

        pDevice = _pDevice;
        pCmdList = NULL;
        pCmdQueue = NULL;
        NodeMask = _NodeMask;

        DescHeaps.InitDescriptorHeaps(pDevice, DescriptorHeaps);

        THROW_IF_FAILED(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence)));
#if _DEBUG
        pFence->SetName(L"SSAOFence");
#endif
        hFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

        FenceValue = 0;
    }

    // This should be called when the library ends
    void Release()
    {
        CloseHandle(hFenceEvent);
        SAFE_RELEASE(pFence);
        DescHeaps.Release();
    }

    void IncrFenceValue()
    {
        FenceValue++;
    }

    void WaitGPUIdle()
    {
        // Schedule a Signal command in the queue.
        THROW_IF_FAILED(pCmdQueue->Signal(pFence, FenceValue));

        THROW_IF_FAILED(pFence->SetEventOnCompletion(FenceValue, hFenceEvent));

        WaitForSingleObjectEx(hFenceEvent, INFINITE, FALSE);
    }

    void AddResourceBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES OldState, D3D12_RESOURCE_STATES NewState)
    {
        if (OldState = NewState)
        {
            return;
        }
        D3D12_RESOURCE_BARRIER BarrierDesc = {};
        BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        BarrierDesc.Transition.pResource = pResource;
        BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        BarrierDesc.Transition.StateBefore = OldState;
        BarrierDesc.Transition.StateAfter = NewState;
        pCmdList->ResourceBarrier(1, &BarrierDesc);
    }
};

struct GFSDK_D3D12_RootParameter
{
    D3D12_ROOT_PARAMETER RootParam;
};

#endif  // SUPPORT_D3D12
