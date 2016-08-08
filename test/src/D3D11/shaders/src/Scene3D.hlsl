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
// Copyright © 2008-2013 NVIDIA Corporation. All rights reserved.
//
// NVIDIA Corporation and its licensors retain all intellectual property and proprietary
// rights in and to this software and related documentation and any modifications thereto.
// Any use, reproduction, disclosure or distribution of this software and related
// documentation without an express license agreement from NVIDIA Corporation is
// strictly prohibited.
//

#pragma pack_matrix( row_major )

#define USE_DDX_DDY 1
#define USE_PER_SAMPLE_SHADING 0

cbuffer GlobalConstantBuffer : register(b0)
{
  float4x4 g_WorldView;
  float4x4 g_WorldViewInverse;
  float4x4 g_WorldViewProjection;
  float  g_IsWhite;
};

Texture2D<float3> tColor    : register(t0);
SamplerState PointSampler   : register(s0);

//-----------------------------------------------------------------------------
// Geometry-rendering shaders
//-----------------------------------------------------------------------------

struct VS_INPUT
{
    float3 WorldPosition            : POSITION;
    float3 WorldNormal              : NORMAL;
};

struct VS_OUTPUT
{
    float4 HPosition                : SV_POSITION;
    centroid float3 ViewPosition    : TEXCOORD0;
    centroid float3 WorldNormal     : TEXCOORD1;
};

struct PS_OUTPUT
{
    float4 Color                    : SV_Target0;
    float3 WorldNormal              : SV_Target1;
};

VS_OUTPUT GeometryVS( VS_INPUT input )
{
    VS_OUTPUT output;
    output.HPosition = mul( float4(input.WorldPosition,1), g_WorldViewProjection );
    output.ViewPosition = mul( float4(input.WorldPosition,1), g_WorldView ).xyz;
    output.WorldNormal = input.WorldNormal.xyz;
    return output;
}

PS_OUTPUT GeometryColorNormalPS( VS_OUTPUT input
#if USE_PER_SAMPLE_SHADING
    , uint SampleIndex : SV_SampleIndex
#endif
    )
{
    PS_OUTPUT output;
    output.Color = g_IsWhite ? float4(1,1,1,1) : float4(.457,.722, 0.0, 1);

#if USE_DDX_DDY
    float3 ViewNormal = normalize(cross(ddx(input.ViewPosition.xyz), ddy(input.ViewPosition.xyz)));
    output.WorldNormal = mul(ViewNormal, (float3x3)g_WorldViewInverse);
#else
    output.WorldNormal = input.WorldNormal;
#endif

    // Lerp to [0,1]^3 to store in the non-signed R11G11B10_FLOAT format
    output.WorldNormal = output.WorldNormal * 0.5 + 0.5;
    return output;
}

float4 GeometryColorPS( VS_OUTPUT input ) : SV_Target0
{
    return g_IsWhite ? float4(1,1,1,1) : float4(.457,.722, 0.0, 1);
}

//-----------------------------------------------------------------------------
// Post-processing shaders
//-----------------------------------------------------------------------------

struct PostProc_VSOut
{
    float4 pos : SV_Position;
    float2 uv  : TexCoord;
};

// Vertex shader that generates a full screen quad with texcoords
// To use draw 3 vertices with primitive type triangle list
PostProc_VSOut FullScreenTriangleVS( uint id : SV_VertexID )
{
    PostProc_VSOut output = (PostProc_VSOut)0.0f;
    output.uv = float2( (id << 1) & 2, id & 2 );
    output.pos = float4( output.uv * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f), 0.0f, 1.0f );
    return output;
}

float4 CopyColorPS( PostProc_VSOut IN ) : SV_TARGET
{
    float3 color = tColor.Sample(PointSampler, IN.uv);
    return float4(color,1);
}
