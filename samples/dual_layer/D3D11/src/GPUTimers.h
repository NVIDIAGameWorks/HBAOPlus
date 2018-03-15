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
#include <d3d11.h>
#include <vector>

typedef int RenderTimeId;

//--------------------------------------------------------------------------------
struct GPUTimerState
{
    bool TimestampQueryInFlight;
    ID3D11Query* pGPUTimersBegin;
    ID3D11Query* pGPUTimersEnd;
    float GPUTimeInMS;
};

//--------------------------------------------------------------------------------
class GPUTimers
{
public:
    void Create(ID3D11Device* pD3DDevice, UINT NumTimers);
    void Release();

    void BeginFrame(ID3D11DeviceContext* pDeviceContext);
    void EndFrame(ID3D11DeviceContext* pDeviceContext);

    void StartTimer(ID3D11DeviceContext* pDeviceContext, UINT Id);
    void StopTimer(ID3D11DeviceContext* pDeviceContext, UINT Id);

    float GetGPUTimeInMS(UINT Id);

protected:
    bool m_DisjointQueryInFlight;
    ID3D11Query* m_pDisjointTimestampQuery;
    std::vector<GPUTimerState> m_Timers;
};

//--------------------------------------------------------------------------------
class GPUTimer
{
public:
    GPUTimer(GPUTimers* pGPUTimers, ID3D11DeviceContext* pDeviceContext, RenderTimeId Id)
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
    ID3D11DeviceContext* m_pDeviceContext;
    RenderTimeId m_RenderTimeId;
};
