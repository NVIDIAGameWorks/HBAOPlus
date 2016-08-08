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

#include "..\..\..\src\Renderer_DX11.h"

#pragma warning( disable : 4996 )
#pragma warning( disable : 4995 )

#define FRAME_COUNT 100
#define SAMPLE_COUNT 5

#if ENABLE_RENDER_TIMES

namespace D3D11
{

class GPUProfiler
{
public:
    enum State
    {
        STATE_INIT,
        STATE_RUN,
        STATE_DONE,
    };

    GPUProfiler()
    : m_State(STATE_INIT)
    , m_FrameId(0)
    , m_SampleId(0)
    {
    }

    bool IsProfiling()
    {
        return (m_State == STATE_RUN);
    }

    bool IsDone()
    {
        return (m_State == STATE_DONE);
    }

    void StartProfiling()
    {
        m_State = STATE_RUN;
        m_FrameId = 0;
    }

    void Tick(const GFSDK::SSAO::RenderTimes &RenderTimes, const char* FilenameCSV)
    {
        if (IsProfiling())
        {
            if (m_FrameId == FRAME_COUNT)
            {
                WriteSample(RenderTimes, FilenameCSV);
            }

            ++m_FrameId;
        }
    }

private:
    State m_State;
    UINT m_FrameId;
    UINT m_SampleId;

    void WriteSample(const GFSDK::SSAO::RenderTimes &RenderTimes, const char* FilenameCSV)
    {
        if (m_SampleId == 0)
        {
            WriteHeader(FilenameCSV);
        }
        AppendTimings(RenderTimes, FilenameCSV);

        if (++m_SampleId == SAMPLE_COUNT)
        {
            m_State = STATE_DONE;
        }
        else
        {
            StartProfiling();
        }
    }

    void WriteHeader(const char* FilenameCSV)
    {
        FILE *fp = fopen(FilenameCSV, "w");
        if (fp)
        {
            fprintf(fp,"Total,LinearizeZ,DeinterleaveZ,NearAO,FarAO,ReinterleaveAO,BlurX,BlurY\n");
            fclose(fp);
        }
    }

    void AppendTimings(const GFSDK::SSAO::RenderTimes &RenderTimes, const char* FilenameCSV)
    {
        FILE *fp = fopen(FilenameCSV, "a+");
        if (fp)
        {
            fprintf(fp, "%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f\n",
                RenderTimes.GPUTimeMS[GFSDK::SSAO::RENDER_PASS_0][GFSDK::SSAO::REGIME_TIME_TOTAL],
                RenderTimes.GPUTimeMS[GFSDK::SSAO::RENDER_PASS_0][GFSDK::SSAO::REGIME_TIME_LINEAR_Z],
                RenderTimes.GPUTimeMS[GFSDK::SSAO::RENDER_PASS_0][GFSDK::SSAO::REGIME_TIME_DEINTERLEAVE_Z],
                RenderTimes.GPUTimeMS[GFSDK::SSAO::RENDER_PASS_0][GFSDK::SSAO::REGIME_TIME_NORMAL],
                RenderTimes.GPUTimeMS[GFSDK::SSAO::RENDER_PASS_0][GFSDK::SSAO::REGIME_TIME_COARSE_AO],
                RenderTimes.GPUTimeMS[GFSDK::SSAO::RENDER_PASS_0][GFSDK::SSAO::REGIME_TIME_INTERLEAVE_AO],
                RenderTimes.GPUTimeMS[GFSDK::SSAO::RENDER_PASS_0][GFSDK::SSAO::REGIME_TIME_BLURX],
                RenderTimes.GPUTimeMS[GFSDK::SSAO::RENDER_PASS_0][GFSDK::SSAO::REGIME_TIME_BLURY]);

            fclose(fp);
        }
    }
};

} //namespace D3D11

#endif //ENABLE_RENDER_TIMES
