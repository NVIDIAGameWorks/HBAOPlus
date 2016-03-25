// This code contains NVIDIA Confidential Information and is disclosed 
// under the Mutual Non-Disclosure Agreement. 
// 
// Notice 
// ALL NVIDIA DESIGN SPECIFICATIONS AND CODE ("MATERIALS") ARE PROVIDED "AS IS" NVIDIA MAKES 
// NO REPRESENTATIONS, WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO 
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ANY IMPLIED WARRANTIES OF NONINFRINGEMENT, 
// MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. 
// 
// NVIDIA Corporation assumes no responsibility for the consequences of use of such 
// information or for any infringement of patents or other rights of third parties that may 
// result from its use. No license is granted by implication or otherwise under any patent 
// or patent rights of NVIDIA Corporation. No third party distribution is allowed unless 
// expressly authorized by NVIDIA.  Details are subject to change without notice. 
// This code supersedes and replaces all information previously supplied. 
// NVIDIA Corporation products are not authorized for use as critical 
// components in life support devices or systems without express written approval of 
// NVIDIA Corporation. 
// 
// Copyright © 2008-2016 NVIDIA Corporation. All rights reserved.
//
// NVIDIA Corporation and its licensors retain all intellectual property and proprietary
// rights in and to this software and related documentation and any modifications thereto.
// Any use, reproduction, disclosure or distribution of this software and related
// documentation without an express license agreement from NVIDIA Corporation is
// strictly prohibited.
//

#include "ConstantBuffers.hlsl"
#include "FullScreenTriangle_VS.hlsl"
#include "FetchNormal_Common.hlsl"

// Disabled as a WAR for nvbug 1370844
#define USE_GATHER4 0

#if API_GL
#define FullResDepthTexture g_t0
#endif

Texture2D<float>  FullResDepthTexture       : register(t0);

sampler           PointClampSampler         : register(s0);

//----------------------------------------------------------------------------------
float3 UVToView(float2 UV, float viewDepth)
{
    UV = g_f2UVToViewA * UV + g_f2UVToViewB;
    return float3(UV * viewDepth, viewDepth);
}

//----------------------------------------------------------------------------------
float3 FetchFullResViewPos(float2 UV)
{
    float ViewDepth = FullResDepthTexture.SampleLevel(PointClampSampler, UV, 0);
    return UVToView(UV, ViewDepth);
}

//----------------------------------------------------------------------------------
float3 MinDiff(float3 P, float3 Pr, float3 Pl)
{
    float3 V1 = Pr - P;
    float3 V2 = P - Pl;
    return (dot(V1,V1) < dot(V2,V2)) ? V1 : V2;
}

//----------------------------------------------------------------------------------
#if USE_GATHER4
float4 GatherR4(Texture2D<float> Texture, float2 UV, int2 o0, int2 o1, int2 o2, int2 o3)
{
    return float4(
        Texture.SampleLevel(PointClampSampler, UV, 0, o0).x,
        Texture.SampleLevel(PointClampSampler, UV, 0, o1).x,
        Texture.SampleLevel(PointClampSampler, UV, 0, o2).x,
        Texture.SampleLevel(PointClampSampler, UV, 0, o3).x
        );
}
#endif

//----------------------------------------------------------------------------------
float3 ReconstructNormal(float2 UV, float3 P)
{
#if USE_GATHER4
    float4 S = GatherR4(FullResDepthTexture, UV, int2(1,0), int2(-1,0), int2(0,1), int2(0,-1));
    float3 Pr = UVToView(float2(UV.x + g_f2InvFullResolution.x, UV.y), S.x);
    float3 Pl = UVToView(float2(UV.x - g_f2InvFullResolution.x, UV.y), S.y);
    float3 Pt = UVToView(float2(UV.x, UV.y + g_f2InvFullResolution.y), S.z);
    float3 Pb = UVToView(float2(UV.x, UV.y - g_f2InvFullResolution.y), S.w);
#else
    float3 Pr = FetchFullResViewPos(UV + float2(g_f2InvFullResolution.x, 0));
    float3 Pl = FetchFullResViewPos(UV + float2(-g_f2InvFullResolution.x, 0));
    float3 Pt = FetchFullResViewPos(UV + float2(0, g_f2InvFullResolution.y));
    float3 Pb = FetchFullResViewPos(UV + float2(0, -g_f2InvFullResolution.y));
#endif
    return normalize(cross(MinDiff(P, Pr, Pl), MinDiff(P, Pt, Pb)));
}
