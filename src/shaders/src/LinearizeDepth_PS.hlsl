/*
#permutation RESOLVE_DEPTH 0 1
#permutation DEPTH_LAYER_COUNT 1 2
*/

/* 
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

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
float ConvertToViewDepth(float HardwareDepth)
{
    float NormalizedDepth = saturate(g_fInverseDepthRangeA * HardwareDepth + g_fInverseDepthRangeB);

    return 1.0 / (NormalizedDepth * g_fLinearizeDepthA + g_fLinearizeDepthB);
}

//----------------------------------------------------------------------------------
#if DEPTH_LAYER_COUNT==2
struct OutputDepths
{
    float ViewDepth0 : SV_Target0;
    float ViewDepth1 : SV_Target1;
};
OutputDepths LinearizeDepth_PS(PostProc_VSOut IN)
{
    AddViewportOrigin(IN);

#if RESOLVE_DEPTH
    float HardwareDepth0 = DepthTextureMS0.Load(int2(IN.pos.xy), g_iSampleIndex);
    float HardwareDepth1 = DepthTextureMS1.Load(int2(IN.pos.xy), g_iSampleIndex);
#else
    float HardwareDepth0 = DepthTexture0.Load(int3(IN.pos.xy, 0));
    float HardwareDepth1 = DepthTexture1.Load(int3(IN.pos.xy, 0));
#endif

    float ViewDepth0 = ConvertToViewDepth(HardwareDepth0);
    float ViewDepth1 = ConvertToViewDepth(HardwareDepth1);

    OutputDepths Out;
    Out.ViewDepth0 = min(ViewDepth0, ViewDepth1);
    Out.ViewDepth1 = max(ViewDepth0, ViewDepth1);
    return Out;
}
#else
float LinearizeDepth_PS(PostProc_VSOut IN) : SV_TARGET
{
    AddViewportOrigin(IN);

#if RESOLVE_DEPTH
    float HardwareDepth = DepthTextureMS.Load(int2(IN.pos.xy), g_iSampleIndex);
#else
    float HardwareDepth = DepthTexture.Load(int3(IN.pos.xy, 0));
#endif

    return ConvertToViewDepth(HardwareDepth);
}
#endif
