/*
#permutation RESOLVE_DEPTH 0 1
*/

/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#include "FullScreenTriangle_VS.hlsl"
#include "ConstantBuffers.hlsl"

#if API_GL
#define DepthTexture    g_t0
#define DepthTextureMS  g_t0
#endif

#if RESOLVE_DEPTH
Texture2DMS<float> DepthTextureMS : register(t0);
#else
Texture2D<float> DepthTexture : register(t0);
#endif

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
