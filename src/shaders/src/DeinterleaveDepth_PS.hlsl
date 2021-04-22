/*
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

#include "ConstantBuffers.hlsl"
#include "FullScreenTriangle_VS.hlsl"

#define USE_GATHER4 1

#define MRT_COUNT MAX_NUM_MRTS

#if DEPTH_LAYER_COUNT==2
Texture2D<float> DepthTexture   : register(t0);
Texture2D<float> DepthTexture2  : register(t1);
#else
Texture2D<float> DepthTexture   : register(t0);
#endif
sampler PointClampSampler       : register(s0);

//----------------------------------------------------------------------------------
#if DEPTH_LAYER_COUNT==2
struct PSOutputDepthTextures
{
    float2 Z00 : SV_Target0;
    float2 Z10 : SV_Target1;
    float2 Z20 : SV_Target2;
    float2 Z30 : SV_Target3;
#if MRT_COUNT == 8
    float2 Z01 : SV_Target4;
    float2 Z11 : SV_Target5;
    float2 Z21 : SV_Target6;
    float2 Z31 : SV_Target7;
#endif
};
#else
struct PSOutputDepthTextures
{
    float Z00 : SV_Target0;
    float Z10 : SV_Target1;
    float Z20 : SV_Target2;
    float Z30 : SV_Target3;
#if MRT_COUNT == 8
    float Z01 : SV_Target4;
    float Z11 : SV_Target5;
    float Z21 : SV_Target6;
    float Z31 : SV_Target7;
#endif
};
#endif

#if USE_GATHER4

//----------------------------------------------------------------------------------
#if DEPTH_LAYER_COUNT==2
PSOutputDepthTextures DeinterleaveDepth_PS(PostProc_VSOut IN)
{
    PSOutputDepthTextures OUT;

    IN.pos.xy = floor(IN.pos.xy) * 4.0 + (g_PerPassConstants.f2Offset + 0.5);
    IN.uv = IN.pos.xy * g_f2InvFullResolution;

    // Gather sample ordering: (-,+),(+,+),(+,-),(-,-),
    float4 S0 = DepthTexture.GatherRed(PointClampSampler, IN.uv);
    float4 S1 = DepthTexture.GatherRed(PointClampSampler, IN.uv, int2(2,0));

    float4 S01 = DepthTexture2.GatherRed(PointClampSampler, IN.uv);
    float4 S11 = DepthTexture2.GatherRed(PointClampSampler, IN.uv, int2(2,0));

    OUT.Z00 = float2(S0.w, S01.w);
    OUT.Z10 = float2(S0.z, S01.z);
    OUT.Z20 = float2(S1.w, S11.w);
    OUT.Z30 = float2(S1.z, S11.z);

#if MRT_COUNT == 8
    OUT.Z01 = float2(S0.x, S01.x);
    OUT.Z11 = float2(S0.y, S01.y);
    OUT.Z21 = float2(S1.x, S11.x);
    OUT.Z31 = float2(S1.y, S11.y);
#endif

    return OUT;
}
#else
PSOutputDepthTextures DeinterleaveDepth_PS(PostProc_VSOut IN)
{
    PSOutputDepthTextures OUT;

    IN.pos.xy = floor(IN.pos.xy) * 4.0 + (g_PerPassConstants.f2Offset + 0.5);
    IN.uv = IN.pos.xy * g_f2InvFullResolution;

    // Gather sample ordering: (-,+),(+,+),(+,-),(-,-),
    float4 S0 = DepthTexture.GatherRed(PointClampSampler, IN.uv);  
    float4 S1 = DepthTexture.GatherRed(PointClampSampler, IN.uv, int2(2,0));

    OUT.Z00 = S0.w;
    OUT.Z10 = S0.z;
    OUT.Z20 = S1.w;
    OUT.Z30 = S1.z;

#if MRT_COUNT == 8
    OUT.Z01 = S0.x;
    OUT.Z11 = S0.y;
    OUT.Z21 = S1.x;
    OUT.Z31 = S1.y;
#endif

    return OUT;
}
#endif

#else

//----------------------------------------------------------------------------------
PSOutputDepthTextures DeinterleaveDepth_PS(PostProc_VSOut IN)
{
    IN.pos.xy = floor(IN.pos.xy) * 4.0 + g_PerPassConstants.f2Offset;
    IN.uv = IN.pos.xy * g_f2InvFullResolution;

    PSOutputDepthTextures OUT;

    OUT.Z00 = DepthTexture.Sample(PointClampSampler, IN.uv);
    OUT.Z10 = DepthTexture.Sample(PointClampSampler, IN.uv, int2(1,0));
    OUT.Z20 = DepthTexture.Sample(PointClampSampler, IN.uv, int2(2,0));
    OUT.Z30 = DepthTexture.Sample(PointClampSampler, IN.uv, int2(3,0));

#if MRT_COUNT == 8
    OUT.Z01 = DepthTexture.Sample(PointClampSampler, IN.uv, int2(0,1));
    OUT.Z11 = DepthTexture.Sample(PointClampSampler, IN.uv, int2(1,1));
    OUT.Z21 = DepthTexture.Sample(PointClampSampler, IN.uv, int2(2,1));
    OUT.Z31 = DepthTexture.Sample(PointClampSampler, IN.uv, int2(3,1));
#endif

    return OUT;
}

#endif
