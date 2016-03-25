/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#if ENABLE_PERF_MARKERS

#pragma once

#if SUPPORT_D3D11
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#endif

#if SUPPORT_D3D12
#include <d3d12.h>
#include <pix_win.h>
#endif

namespace GFSDK
{
namespace SSAO
{

//--------------------------------------------------------------------------------
#if SUPPORT_D3D11
namespace D3D11
{

struct PerfMarkerScope
{
    PerfMarkerScope(const WCHAR* EventName)
    {
        D3DPERF_BeginEvent(0xff808080, EventName);
    }
    ~PerfMarkerScope()
    {
        D3DPERF_EndEvent();
    }
};

#define PERF_MARKER_SCOPE(NAME) PerfMarkerScope PerfMarker(NAME)

} // namespace D3D11
#endif //SUPPORT_D3D11

//--------------------------------------------------------------------------------
#if SUPPORT_D3D12
namespace D3D12
{

struct PerfMarkerScope
{
    PerfMarkerScope()
        : m_pCmdList(NULL)
    {
    }
    PerfMarkerScope(ID3D12GraphicsCommandList* pCmdList, const WCHAR* EventName)
    {
        PIXBeginEvent(pCmdList, 0, EventName);

        m_pCmdList = pCmdList;
    }
    ~PerfMarkerScope()
    {
        PIXEndEvent(m_pCmdList);

        m_pCmdList = NULL;
    }

private:
    ID3D12GraphicsCommandList* m_pCmdList;
};

#define PERF_MARKER_SCOPE(NAME) PerfMarkerScope PerfMarker(m_GraphicsContext.pCmdList, NAME)

} // namespace D3D12
#endif //SUPPORT_D3D12

} // namespace SSAO
} // namespace GFSDK

#endif //ENABLE_PERF_MARKERS
