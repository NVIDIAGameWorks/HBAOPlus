/*
#permutation ENABLE_BLUR 0 1
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

#include "ConstantBuffers.hlsl"
#include "FullScreenTriangle_VS.hlsl"

#define USE_INTEGER_MATH 1

Texture2DArray<float> AOTexture     : register(t0);
#if DEPTH_LAYER_COUNT==2
Texture2D<float> DepthTexture1      : register(t1);
Texture2D<float> DepthTexture2      : register(t2);
#else
Texture2D<float> DepthTexture       : register(t1);
#endif

sampler PointSampler                : register(s0);

//----------------------------------------------------------------------------------
struct PSOut
{
#if ENABLE_BLUR
    float2 AOZ  : SV_TARGET;
#else
    float4 AO   : SV_TARGET;
#endif
};

//-------------------------------------------------------------------------
PSOut ReinterleaveAO_PS(PostProc_VSOut IN)
{
    PSOut OUT;

#if !ENABLE_BLUR
    SubtractViewportOrigin(IN);
#endif

#if USE_INTEGER_MATH
    int2 FullResPos = int2(IN.pos.xy);
    int2 Offset = FullResPos & 3;
    int SliceId = Offset.y * 4 + Offset.x;
    int2 QuarterResPos = FullResPos >> 2;
#else
    float2 FullResPos = floor(IN.pos.xy);
    float2 Offset = fmod(abs(FullResPos), float2(4,4));
    float SliceId = Offset.y * 4.0 + Offset.x;
    float2 QuarterResPos = FullResPos / 4.0;
#endif


    float AO = AOTexture.Load(int4(QuarterResPos, SliceId, 0));
#if ENABLE_BLUR
#if DEPTH_LAYER_COUNT==2
    float ViewDepth1 = DepthTexture1.Sample(PointSampler, IN.uv);
    float ViewDepth2 = DepthTexture2.Sample(PointSampler, IN.uv);
    float ViewDepth = min(ViewDepth1, ViewDepth2);
#else
    float ViewDepth = DepthTexture.Sample(PointSampler, IN.uv);
#endif
    OUT.AOZ = float2(AO, ViewDepth);
#else    
    OUT.AO = pow(saturate(AO), g_fPowExponent);
#endif

    return OUT;
}
