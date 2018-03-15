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
#pragma warning( disable : 4995 )
#include <d3d12.h>
#include <vector>

typedef int RenderTimeId;

//--------------------------------------------------------------------------------
struct GPUTimerState
{
    bool TimestampQueryInFlight;
    float GPUTimeInMS;
};

//--------------------------------------------------------------------------------
class GPUTimers
{
public:
    void Create(ID3D12Device* pD3DDevice, ID3D12CommandQueue* pQueue, UINT NumTimers);
    void Release();

    void BeginFrame(ID3D12GraphicsCommandList* pCommandList);
    void EndFrame(ID3D12GraphicsCommandList* pCommandList);

    void StartTimer(ID3D12GraphicsCommandList* pCommandList, UINT Id);
    void StopTimer(ID3D12GraphicsCommandList* pCommandList, UINT Id);

    float GetGPUTimeInMS(UINT Id);

protected:
    ID3D12QueryHeap* m_pQueryHeap;
    ID3D12Resource*  m_pQueryResults;
    ID3D12CommandQueue* m_pQueue;

    std::vector<GPUTimerState>  m_Timers;
};

//--------------------------------------------------------------------------------
class GPUTimer
{
public:
    GPUTimer(GPUTimers* pGPUTimers, ID3D12GraphicsCommandList* pDeviceContext, RenderTimeId Id)
        : m_pGPUTimers(pGPUTimers)
        , m_pDeviceContext(pDeviceContext)
        , m_RenderTimeId(Id)
    {
        m_pGPUTimers->StartTimer(m_pDeviceContext, m_RenderTimeId);
    }
    ~GPUTimer()
    {
        m_pGPUTimers->StopTimer(m_pDeviceContext, m_RenderTimeId);
    }

private:
    GPUTimers* m_pGPUTimers;
    ID3D12GraphicsCommandList* m_pDeviceContext;
    RenderTimeId m_RenderTimeId;
};
