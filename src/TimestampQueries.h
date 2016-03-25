/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
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

enum GpuTimeRenderPassIndex
{
    RENDER_PASS_0,
    RENDER_PASS_1,
    RENDER_PASS_COUNT,
};

struct RenderTimes
{
    static GpuTimeRenderPassIndex SSAO::RenderTimes::CurrentPassIndex;
    float GPUTimeMS[RENDER_PASS_COUNT][REGIME_TIME_COUNT];
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
                            pRenderTimes->GPUTimeMS[GFSDK::SSAO::RenderTimes::CurrentPassIndex][i] = float(double(TimestampValueEnd - TimestampValueBegin) * InvFrequencyMS);
                        }
                    }
                    else
                    {
                        pRenderTimes->GPUTimeMS[GFSDK::SSAO::RenderTimes::CurrentPassIndex][i] = 0.f;
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

#if SUPPORT_GL

namespace GL
{

#if defined(_MSC_VER) && _MSC_VER < 1400
typedef __int64 GLint64EXT;
typedef unsigned __int64 GLuint64EXT;
#elif defined(_MSC_VER) || defined(__BORLANDC__)
typedef signed long long GLint64EXT;
typedef unsigned long long GLuint64EXT;
#else
#  if defined(__MINGW32__) || defined(__CYGWIN__)
#include <inttypes.h>
#  endif
typedef int64_t GLint64EXT;
typedef uint64_t GLuint64EXT;
#endif
typedef GLint64EXT  GLint64;
typedef GLuint64EXT GLuint64;

//--------------------------------------------------------------------------------
struct TimerQuery
{
    GLuint Query;
    GLint64EXT Duration;
    bool InFlight;
};

//--------------------------------------------------------------------------------
class TimestampQueries
{
public:
    typedef struct
    {
        void (GFSDK_SSAO_STDCALL * glGenQueries) (GLsizei n, GLuint* ids);
        void (GFSDK_SSAO_STDCALL * glDeleteQueries) (GLsizei n, const GLuint* ids);
        void (GFSDK_SSAO_STDCALL * glGetQueryObjectiv) (GLuint id, GLenum pname, GLint* params);
        void (GFSDK_SSAO_STDCALL * glGetQueryObjecti64vEXT) (GLuint id, GLenum pname, GLint64EXT *params);
        void (GFSDK_SSAO_STDCALL * glBeginQuery) (GLenum target, GLuint id);
        void (GFSDK_SSAO_STDCALL * glEndQuery) (GLenum target);
    } GLFunctions;

    static GLFunctions GL;

    void Create()
    {
        for (UINT i = 0; i < REGIME_TIME_COUNT; i++)
        {
            GL.glGenQueries(1, &m_TimerQueries[i].Query);
            m_TimerQueries[i].Duration = 0;
            m_TimerQueries[i].InFlight = false;
        }
    }

    void Release()
    {
        for (UINT i = 0; i < REGIME_TIME_COUNT; i++)
        {
            GL.glDeleteQueries(1, &m_TimerQueries[i].Query);
            m_TimerQueries[i].Query = 0;
            m_TimerQueries[i].Duration = 0;
            m_TimerQueries[i].InFlight = false;
        }
    }

    void UpdateTotalGpuTime(SSAO::RenderTimes* pRenderTimes)
    {
        pRenderTimes->GPUTimeMS[SSAO::RenderTimes::CurrentPassIndex][REGIME_TIME_TOTAL] = 0.f;

        for (UINT i = 0; i < REGIME_TIME_COUNT; ++i)
        {
            if (i != REGIME_TIME_TOTAL)
            {
                pRenderTimes->GPUTimeMS[SSAO::RenderTimes::CurrentPassIndex][REGIME_TIME_TOTAL] += pRenderTimes->GPUTimeMS[SSAO::RenderTimes::CurrentPassIndex][i];
            }
        }
    }

    void GetAvailableTimers(SSAO::RenderTimes* pRenderTimes)
    {
        for (UINT i = 0; i < REGIME_TIME_COUNT; ++i)
        {
            if (m_TimerQueries[i].InFlight)
            {
                GLint Available = 0;
                GL.glGetQueryObjectiv(m_TimerQueries[i].Query, GL_QUERY_RESULT_AVAILABLE, &Available);

                if (Available)
                {
                    GL.glGetQueryObjecti64vEXT(m_TimerQueries[i].Query, GL_QUERY_RESULT, &m_TimerQueries[i].Duration);
                    pRenderTimes->GPUTimeMS[SSAO::RenderTimes::CurrentPassIndex][i] = float(double(m_TimerQueries[i].Duration * 1.e-6));
                    m_TimerQueries[i].InFlight = false;
                }
            }
            else
            {
                pRenderTimes->GPUTimeMS[SSAO::RenderTimes::CurrentPassIndex][i] = 0.f;
            }
        }

        UpdateTotalGpuTime(pRenderTimes);
    }

    void StartTimer(GpuTimeRegimeIndex Id)
    {
        if (!m_TimerQueries[Id].InFlight)
        {
            GL.glBeginQuery(GL_TIME_ELAPSED_EXT, m_TimerQueries[Id].Query);
        }
    }

    void StopTimer(GpuTimeRegimeIndex Id)
    {
        if (!m_TimerQueries[Id].InFlight)
        {
            GL.glEndQuery(GL_TIME_ELAPSED_EXT);
        }
        m_TimerQueries[Id].InFlight = true;
    }

private:
    TimerQuery m_TimerQueries[REGIME_TIME_COUNT];
};

//--------------------------------------------------------------------------------
class GPUTimer
{
public:
    GPUTimer(TimestampQueries* pTimestampQueries, GpuTimeRegimeIndex Id)
        : m_pTimestampQueries(pTimestampQueries)
        , m_GpuTimeRegimeIndex(Id)
    {
        m_pTimestampQueries->StartTimer(Id);
    }

    ~GPUTimer()
    {
        m_pTimestampQueries->StopTimer(m_GpuTimeRegimeIndex);
    }

private:
    TimestampQueries* m_pTimestampQueries;
    GpuTimeRegimeIndex m_GpuTimeRegimeIndex;
};

} //namespace GL

#endif //SUPPORT_GL

} // namespace SSAO
} // namespace GFSDK

#endif // ENABLE_RENDER_TIMES
