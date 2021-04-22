/*
#permutation RESOLVE_DEPTH 0 1
#permutation DEPTH_LAYER_COUNT 1 2
*/

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

#include "FullScreenTriangle_VS.hlsl"
#include "ConstantBuffers.hlsl"

#if DEPTH_LAYER_COUNT==2
    #if RESOLVE_DEPTH
        Texture2DMS<float> DepthTextureMS0 : register(t0);
        Texture2DMS<float> DepthTextureMS1 : register(t1);
    #else
        Texture2D<float> DepthTexture0 : register(t0);
        Texture2D<float> DepthTexture1 : register(t1);
    #endif
#else
    #if RESOLVE_DEPTH
        Texture2DMS<float> DepthTextureMS : register(t0);
    #else
        Texture2D<float> DepthTexture : register(t0);
    #endif
#endif

//----------------------------------------------------------------------------------
#if DEPTH_LAYER_COUNT==2
struct OutputDepths
{
    float ViewDepth0 : SV_Target0;
    float ViewDepth1 : SV_Target1;
};
OutputDepths CopyDepth_PS(PostProc_VSOut IN)
{
    AddViewportOrigin(IN);

#if RESOLVE_DEPTH
    float ViewDepth0 = DepthTextureMS0.Load(int2(IN.pos.xy), g_iSampleIndex);
    float ViewDepth1 = DepthTextureMS1.Load(int2(IN.pos.xy), g_iSampleIndex);
#else
    float ViewDepth0 = DepthTexture0.Load(int3(IN.pos.xy, 0));
    float ViewDepth1 = DepthTexture1.Load(int3(IN.pos.xy, 0));
#endif

    OutputDepths Out;
    Out.ViewDepth0 = min(ViewDepth0, ViewDepth1);
    Out.ViewDepth1 = max(ViewDepth0, ViewDepth1);
    return Out;
}
#else
//----------------------------------------------------------------------------------
float CopyDepth_PS(PostProc_VSOut IN) : SV_TARGET
{
    AddViewportOrigin(IN);

#if RESOLVE_DEPTH
    float ViewDepth = DepthTextureMS.Load(int2(IN.pos.xy), g_iSampleIndex);
#else
    float ViewDepth = DepthTexture.Load(int3(IN.pos.xy, 0));
#endif

    return ViewDepth;
}
#endif
