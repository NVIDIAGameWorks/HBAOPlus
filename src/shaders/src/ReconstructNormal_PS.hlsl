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
    return ReconstructNormal(IN.uv, ViewPosition);
}

//----------------------------------------------------------------------------------
float4 ReconstructNormal_PS(PostProc_VSOut IN) : SV_TARGET
{
    float3 ViewPosition = FetchFullResViewPos(IN.uv);
    float3 ViewNormal = FetchFullResViewNormal(IN, ViewPosition);

    return float4(ViewNormal * 0.5 + 0.5, 0);
}
