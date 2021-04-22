// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2021 NVIDIA Corporation. All rights reserved.

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
