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
#include "Common.h"
#include "InputViewport.h"

namespace GFSDK
{
namespace SSAO
{

//--------------------------------------------------------------------------------
struct Viewports
{
    void SetFullResolution(UINT FullWidth, UINT FullHeight)
    {
        FullRes.TopLeftX = 0.f;
        FullRes.TopLeftY = 0.f;
        FullRes.Width    = FLOAT(FullWidth);
        FullRes.Height   = FLOAT(FullHeight);
        FullRes.MinDepth = 0.f;
        FullRes.MaxDepth = 1.f;

        QuarterRes           = FullRes;
        QuarterRes.Width     = FLOAT(iDivUp(FullWidth,4));
        QuarterRes.Height    = FLOAT(iDivUp(FullHeight,4));
    }

    SSAO::InputViewport FullRes;
    SSAO::InputViewport QuarterRes;
};

} // namespace SSAO
} // namespace GFSDK
