/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#include "GPUTimers.h"
#include <assert.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef SAFE_D3D_CALL
#define SAFE_D3D_CALL(x)     { if (x != S_OK) assert(0); }
#endif

//--------------------------------------------------------------------------------
void GPUTimers::Create(ID3D11Device* pD3DDevice, UINT NumTimers)
{
    m_Timers.resize(NumTimers);

    D3D11_QUERY_DESC queryDesc;
    queryDesc.MiscFlags = 0;

    queryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
    SAFE_D3D_CALL( pD3DDevice->CreateQuery(&queryDesc, &m_pDisjointTimestampQuery) );
    m_DisjointQueryInFlight = false;

    queryDesc.Query = D3D11_QUERY_TIMESTAMP;
    for (UINT i = 0; i < m_Timers.size(); ++i)
    {
        SAFE_D3D_CALL( pD3DDevice->CreateQuery(&queryDesc, &m_Timers[i].pGPUTimersBegin) );
        SAFE_D3D_CALL( pD3DDevice->CreateQuery(&queryDesc, &m_Timers[i].pGPUTimersEnd) );
        m_Timers[i].TimestampQueryInFlight = false;
    }
}

//--------------------------------------------------------------------------------
void GPUTimers::Release()
{
    SAFE_RELEASE(m_pDisjointTimestampQuery);

    for (UINT i = 0; i < m_Timers.size(); ++i)
    {
        SAFE_RELEASE(m_Timers[i].pGPUTimersBegin);
        SAFE_RELEASE(m_Timers[i].pGPUTimersEnd);
    }

    m_Timers.clear();
}

//--------------------------------------------------------------------------------
void GPUTimers::BeginFrame(ID3D11DeviceContext* pDeviceContext)
{
    if (!m_DisjointQueryInFlight)
    {
        pDeviceContext->Begin(m_pDisjointTimestampQuery);
    }
}

//--------------------------------------------------------------------------------
void GPUTimers::EndFrame(ID3D11DeviceContext* pDeviceContext)
{
    if (!m_DisjointQueryInFlight)
    {
        pDeviceContext->End(m_pDisjointTimestampQuery);
    }
    m_DisjointQueryInFlight = true;

    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointTimestampValue;
    if (pDeviceContext->GetData(m_pDisjointTimestampQuery, &disjointTimestampValue, sizeof(disjointTimestampValue), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK)
    {
        m_DisjointQueryInFlight = false;

        if (!disjointTimestampValue.Disjoint)
        {
            double InvFrequencyMS = 1000.0 / disjointTimestampValue.Frequency;
            for (UINT i = 0; i < m_Timers.size(); ++i)
            {
                if (m_Timers[i].TimestampQueryInFlight)
                {
                    UINT64 TimestampValueBegin;
                    UINT64 TimestampValueEnd;
                    if ((pDeviceContext->GetData(m_Timers[i].pGPUTimersBegin, &TimestampValueBegin, sizeof(UINT64), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK) &&
                        (pDeviceContext->GetData(m_Timers[i].pGPUTimersEnd,   &TimestampValueEnd,   sizeof(UINT64), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK))
                    {
                        m_Timers[i].TimestampQueryInFlight = false;
                        m_Timers[i].GPUTimeInMS = float(double(TimestampValueEnd - TimestampValueBegin) * InvFrequencyMS);
                    }
                }
                else
                {
                    m_Timers[i].GPUTimeInMS = 0.f;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------
void GPUTimers::StartTimer(ID3D11DeviceContext* pDeviceContext, UINT i)
{
    if (!m_Timers[i].TimestampQueryInFlight)
    {
        pDeviceContext->End(m_Timers[i].pGPUTimersBegin);
    }
}

//--------------------------------------------------------------------------------
void GPUTimers::StopTimer(ID3D11DeviceContext* pDeviceContext, UINT i)
{
    if (!m_Timers[i].TimestampQueryInFlight)
    {
        pDeviceContext->End(m_Timers[i].pGPUTimersEnd);
    }
    m_Timers[i].TimestampQueryInFlight = true;
}

//--------------------------------------------------------------------------------
float GPUTimers::GetGPUTimeInMS(UINT i)
{
    if (i < m_Timers.size())
    {
        return m_Timers[i].GPUTimeInMS;
    }
    assert(0);
    return 0.f;
}
