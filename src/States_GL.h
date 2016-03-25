/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#if SUPPORT_GL

#pragma once
#include "Common.h"

namespace GFSDK
{
namespace SSAO
{
namespace GL
{

class States
{
public:
    static void SetSharedBlendState(const GFSDK_SSAO_GLFunctions& GL);
    static void SetBlendStateMultiplyPreserveAlpha(const GFSDK_SSAO_GLFunctions& GL);
    static void SetBlendStateDisabledPreserveAlpha(const GFSDK_SSAO_GLFunctions& GL);
    static void SetBlendStateDisabled(const GFSDK_SSAO_GLFunctions& GL);
    static void SetDepthStencilStateDisabled(const GFSDK_SSAO_GLFunctions& GL);
    static void SetRasterizerStateFullscreenNoScissor(const GFSDK_SSAO_GLFunctions& GL);
    static void SetCustomBlendState(const GFSDK_SSAO_GLFunctions& GL, const GFSDK_SSAO_CustomBlendState_GL& CustomBlendState);
};

} // namespace GL
} // namespace SSAO
} // namespace GFSDK

#endif // SUPPORT_GL
