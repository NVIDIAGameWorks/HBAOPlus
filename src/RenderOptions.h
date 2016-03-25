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

namespace GFSDK
{
namespace SSAO
{

//--------------------------------------------------------------------------------
struct RenderOptions
{
    void SetRenderOptions(const GFSDK_SSAO_Parameters& Params)
    {
        DepthStorage = Params.DepthStorage;
        DepthClampMode = Params.DepthClampMode;
        Blur = Params.Blur;
        EnableForegroundAO = Params.ForegroundAO.Enable;
        EnableBackgroundAO = Params.BackgroundAO.Enable;
        EnableDepthThreshold = Params.DepthThreshold.Enable;
    }

    GFSDK_SSAO_DepthStorage DepthStorage;
    GFSDK_SSAO_DepthClampMode DepthClampMode;
    GFSDK_SSAO_BlurParameters Blur;
    GFSDK_SSAO_BOOL EnableForegroundAO;
    GFSDK_SSAO_BOOL EnableBackgroundAO;
    GFSDK_SSAO_BOOL EnableDepthThreshold;
};

} // namespace SSAO
} // namespace GFSDK
