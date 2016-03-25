/*
#permutation FETCH_GBUFFER_NORMAL 0 1 2
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

#include "ReconstructNormal_Common.hlsl"

//----------------------------------------------------------------------------------
float3 FetchFullResViewNormal(PostProc_VSOut IN, float3 ViewPosition)
{
#if FETCH_GBUFFER_NORMAL
    return FetchFullResViewNormal_GBuffer(IN);
#else
    return ReconstructNormal(IN.uv, ViewPosition);
#endif
}

//----------------------------------------------------------------------------------
float4 DebugNormals_PS(PostProc_VSOut IN) : SV_TARGET
{
    SubtractViewportOrigin(IN);

    float3 ViewPosition = FetchFullResViewPos(IN.uv);
    float3 ViewNormal = -FetchFullResViewNormal(IN, ViewPosition);

    float3 OutColor = 
        (g_iDebugNormalComponent == 0) ? ViewNormal.xxx :
        (g_iDebugNormalComponent == 1) ? ViewNormal.yyy :
        (g_iDebugNormalComponent == 2) ? ViewNormal.zzz :
        ViewNormal;

    return float4(OutColor, 0);
}
