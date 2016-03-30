/*
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "ConstantBuffers.hlsl"

#if API_GL
#define FullResNormalTexture    g_t1
#define FullResNormalTextureMS  g_t1
#endif

#if FETCH_GBUFFER_NORMAL == 2
Texture2DMS<float3> FullResNormalTextureMS  : register(t1);
#else
Texture2D<float3> FullResNormalTexture      : register(t1);
#endif

//----------------------------------------------------------------------------------
float3 FetchFullResWorldNormal_GBuffer(PostProc_VSOut IN)
{
    AddViewportOrigin(IN);

#if FETCH_GBUFFER_NORMAL == 2
    return FullResNormalTextureMS.Load(int2(IN.pos.xy), g_iSampleIndex).xyz;
#else
    return FullResNormalTexture.Load(int3(IN.pos.xy, 0)).xyz;
#endif
}

//----------------------------------------------------------------------------------
float3 FetchFullResViewNormal_GBuffer(PostProc_VSOut IN)
{
    float3 WorldNormal = FetchFullResWorldNormal_GBuffer(IN) * g_fNormalDecodeScale + g_fNormalDecodeBias;
    float3 ViewNormal = normalize(mul(WorldNormal, (float3x3)g_f44NormalMatrix));
    return ViewNormal;
}
