/* 
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#if ENABLE_RENDER_TIMES

#pragma once
#include "Common.h"

namespace GFSDK
{
namespace SSAO
{

//--------------------------------------------------------------------------------

enum GpuTimeRegimeIndex
{
    REGIME_TIME_LINEAR_Z,
    REGIME_TIME_DEINTERLEAVE_Z,
    REGIME_TIME_NORMAL,
    REGIME_TIME_COARSE_AO,
    REGIME_TIME_INTERLEAVE_AO,
    REGIME_TIME_BLURX,
    REGIME_TIME_BLURY,
    REGIME_TIME_TOTAL,
    REGIME_TIME_COUNT
};

struct RenderTimes
{
    float GPUTimeMS[REGIME_TIME_COUNT];
};

#if SUPPORT_D3D11

namespace D3D11
{

//--------------------------------------------------------------------------------
class TimestampQueries
{
public:
    void Create(ID3D11Device* pD3DDevice)
    {
        D3D11_QUERY_DESC queryDesc;
        queryDesc.MiscFlags = 0;

        queryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        SAFE_D3D_CALL( pD3DDevice->CreateQuery(&queryDesc, &m_pDisjointTimestampQuery) );
        m_DisjointQueryInFlight = false;

        queryDesc.Query = D3D11_QUERY_TIMESTAMP;
        for (UINT i = 0; i < REGIME_TIME_COUNT; ++i)
        {
            SAFE_D3D_CALL( pD3DDevice->CreateQuery(&queryDesc, &m_pTimestampQueriesBegin[i]) );
            SAFE_D3D_CALL( pD3DDevice->CreateQuery(&queryDesc, &m_pTimestampQueriesEnd[i]) );
            m_TimestampQueryInFlight[i] = false;
        }
    }

    void Release()
    {
        SAFE_RELEASE(m_pDisjointTimestampQuery);

        for (UINT i = 0; i < REGIME_TIME_COUNT; ++i)
        {
            SAFE_RELEASE(m_pTimestampQueriesBegin[i]);
            SAFE_RELEASE(m_pTimestampQueriesEnd[i]);
        }
    }

    void Begin(ID3D11DeviceContext* pDeviceContext)
    {
        if (!m_DisjointQueryInFlight)
        {
            pDeviceContext->Begin(m_pDisjointTimestampQuery);
        }
    }

    void End(ID3D11DeviceContext* pDeviceContext, SSAO::RenderTimes* pRenderTimes)
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
                for (UINT i = 0; i < REGIME_TIME_COUNT; ++i)
                {
                    if (m_TimestampQueryInFlight[i])
                    {
                        UINT64 TimestampValueBegin;
                        UINT64 TimestampValueEnd;
                        if ((pDeviceContext->GetData(m_pTimestampQueriesBegin[i], &TimestampValueBegin, sizeof(UINT64), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK) &&
                            (pDeviceContext->GetData(m_pTimestampQueriesEnd[i],   &TimestampValueEnd,   sizeof(UINT64), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK))
                        {
                            m_TimestampQueryInFlight[i] = false;
                            pRenderTimes->GPUTimeMS[i] = float(double(TimestampValueEnd - TimestampValueBegin) * InvFrequencyMS);
                        }
                    }
                    else
                    {
                        pRenderTimes->GPUTimeMS[i] = 0.f;
                    }
                }
            }
        }
    }

    void StartTimer(ID3D11DeviceContext* pDeviceContext, GpuTimeRegimeIndex Id)
    {
        if (!m_TimestampQueryInFlight[Id])
        {
            pDeviceContext->End(m_pTimestampQueriesBegin[Id]);
        }
    }

    void StopTimer(ID3D11DeviceContext* pDeviceContext, GpuTimeRegimeIndex Id)
    {
        if (!m_TimestampQueryInFlight[Id])
        {
            pDeviceContext->End(m_pTimestampQueriesEnd[Id]);
        }
        m_TimestampQueryInFlight[Id] = true;
    }

private:
    bool m_DisjointQueryInFlight;
    bool m_TimestampQueryInFlight[REGIME_TIME_COUNT];
    ID3D11Query* m_pDisjointTimestampQuery;
    ID3D11Query* m_pTimestampQueriesBegin[REGIME_TIME_COUNT];
    ID3D11Query* m_pTimestampQueriesEnd[REGIME_TIME_COUNT];
};

//--------------------------------------------------------------------------------
class GPUTimer
{
public:
    GPUTimer(TimestampQueries* pTimestampQueries, ID3D11DeviceContext* pDeviceContext, GpuTimeRegimeIndex Id)
        : m_pTimestampQueries(pTimestampQueries)
        , m_pDeviceContext(pDeviceContext)
        , m_GpuTimeRegimeIndex(Id)
    {
        m_pTimestampQueries->StartTimer(m_pDeviceContext, m_GpuTimeRegimeIndex);
    }

    ~GPUTimer()
    {
        m_pTimestampQueries->StopTimer(m_pDeviceContext, m_GpuTimeRegimeIndex);
    }

private:
    TimestampQueries* m_pTimestampQueries;
    ID3D11DeviceContext* m_pDeviceContext;
    GpuTimeRegimeIndex m_GpuTimeRegimeIndex;
};

#define GPU_TIMER_SCOPE(NAME) GPUTimer Timer(&m_TimestampQueries, pDeviceContext, GFSDK::SSAO::REGIME_TIME_ ## NAME)

} //namespace D3D11

#endif //SUPPORT_D3D11

} // namespace SSAO
} // namespace GFSDK

#endif // ENABLE_RENDER_TIMES
