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
#include "FullScreenTriangle_VS.hlsl"

struct GSOut
{
    float4 pos  : SV_Position;
    float2 uv   : TEXCOORD0;
    uint LayerIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3)]
void CoarseAO_GS(triangle PostProc_VSOut input[3], inout TriangleStream<GSOut> OUT)
{
    GSOut OutVertex;

    OutVertex.LayerIndex = g_PerPassConstants.uSliceIndex;

    [unroll]
    for (int VertexID = 0; VertexID < 3; VertexID++)
    {
        OutVertex.uv  = input[VertexID].uv;
        OutVertex.pos = input[VertexID].pos;
        OUT.Append(OutVertex);
    }
}
