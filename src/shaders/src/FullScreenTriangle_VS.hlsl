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

struct PostProc_VSOut
{
    float4 pos  : SV_Position;
    float2 uv   : TEXCOORD0;
};

//----------------------------------------------------------------------------------
// Vertex shader that generates a full-screen triangle with texcoords
// Assuming a Draw(3,0) call.
//----------------------------------------------------------------------------------
PostProc_VSOut FullScreenTriangle_VS( uint VertexId : SV_VertexID )
{
    PostProc_VSOut output = (PostProc_VSOut)0.0f;
    output.uv = float2( (VertexId << 1) & 2, VertexId & 2 );
    output.pos = float4( output.uv * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f) , 0.0f, 1.0f );
    return output;
}

//----------------------------------------------------------------------------------
void AddViewportOrigin(inout PostProc_VSOut IN)
{
    IN.pos.xy += g_f2InputViewportTopLeft;
    IN.uv = IN.pos.xy * g_f2InvFullResolution;
}

//----------------------------------------------------------------------------------
void SubtractViewportOrigin(inout PostProc_VSOut IN)
{
    IN.pos.xy -= g_f2InputViewportTopLeft;
    IN.uv = IN.pos.xy * g_f2InvFullResolution;
}
