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

#pragma once
#include "Common.h"

namespace GFSDK
{
namespace SSAO
{
namespace D3D11
{

#if SUPPORT_D3D11

//--------------------------------------------------------------------------------
class AppState
{
public:
    AppState()
    {
        memset(this, 0, sizeof(*this));
    }

    void Save(ID3D11DeviceContext* pDeviceContext);
    void Restore(ID3D11DeviceContext* pDeviceContext);
    static void UnbindSRVs(ID3D11DeviceContext* pDeviceContext);

private:
    static const int NumPSShaderResourceViews = 4;
    static const int NumPSSamplers = 2;
    static const int NumGSConstantBuffers = 4;
    static const int NumPSConstantBuffers = 4;
    static const int NumRenderTargetViews = 8;

    D3D11_VIEWPORT Viewport;
    D3D11_PRIMITIVE_TOPOLOGY Topology;
    ID3D11InputLayout* pInputLayout;
    ID3D11RasterizerState* pRasterizerState;

    ID3D11VertexShader* pVS;
    ID3D11HullShader* pHS;
    ID3D11DomainShader* pDS;
    ID3D11GeometryShader* pGS;
    ID3D11PixelShader* pPS;
    ID3D11ComputeShader* pCS;

    ID3D11ShaderResourceView* pPSShaderResourceViews[NumPSShaderResourceViews];
    ID3D11SamplerState* pPSSamplers[NumPSSamplers];
    ID3D11Buffer* pGSConstantBuffers[NumGSConstantBuffers];
    ID3D11Buffer* pPSConstantBuffers[NumPSConstantBuffers];

    ID3D11DepthStencilState* pDepthStencilState;
    UINT StencilRef;
    ID3D11BlendState* pBlendState;
    FLOAT BlendFactor[4];
    UINT SampleMask;
    ID3D11RenderTargetView* pRenderTargetViews[NumRenderTargetViews];
    ID3D11DepthStencilView* pDepthStencilView;
};

#endif // SUPPORT_D3D11

} // namespace D3D11
} // namespace SSAO
} // namespace GFSDK
