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

#if SUPPORT_D3D12

#include "States_DX12.h"

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::States::CreateBlendStates()
{
    //
    // Create BlendState_Disabled
    //

    D3D12_BLEND_DESC BlendStateDesc = {};
    BlendStateDesc.AlphaToCoverageEnable = FALSE;
    BlendStateDesc.IndependentBlendEnable = TRUE;

    for (UINT i = 0; i < SIZEOF_ARRAY(BlendStateDesc.RenderTarget); ++i)
    {
        BlendStateDesc.RenderTarget[i].BlendEnable = FALSE;
        BlendStateDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }

    m_pBlendState_Disabled = BlendStateDesc;

    //
    // Create BlendState_Multiply_PreserveAlpha
    //

    BlendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE;
    BlendStateDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
    BlendStateDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
    BlendStateDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    BlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
    BlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    BlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    m_pBlendState_Multiply_PreserveAlpha = BlendStateDesc;

    //
    // Create BlendState_Disabled_PreserveAlpha
    //

    BlendStateDesc.RenderTarget[0].BlendEnable = FALSE;

    m_pBlendState_Disabled_PreserveAlpha = BlendStateDesc;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::States::CreateDepthStencilStates()
{
    //
    // Create DepthStencilState_Disabled
    //

    static D3D12_DEPTH_STENCIL_DESC DepthStencilDesc =
    { 0x0, //DepthEnable
    D3D12_DEPTH_WRITE_MASK_ZERO, //DepthWriteMask
    D3D12_COMPARISON_FUNC_NEVER, //DepthFunc
    0x0, //StencilEnable
    0xFF, //StencilReadMask
    0xFF, //StencilWriteMask

    { D3D12_STENCIL_OP_KEEP, //StencilFailOp
    D3D12_STENCIL_OP_KEEP, //StencilDepthFailOp
    D3D12_STENCIL_OP_KEEP, //StencilPassOp
    D3D12_COMPARISON_FUNC_ALWAYS  //StencilFunc
    }, //FrontFace

    { D3D12_STENCIL_OP_KEEP, //StencilFailOp
    D3D12_STENCIL_OP_KEEP, //StencilDepthFailOp
    D3D12_STENCIL_OP_KEEP, //StencilPassOp
    D3D12_COMPARISON_FUNC_ALWAYS  //StencilFunc
    }  //BackFace
    };

    m_pDepthStencilState_Disabled = DepthStencilDesc;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::States::CreateRasterizerStates()
{
    //
    // Create RasterizerState_Fullscreen_NoScissor
    //

    static D3D12_RASTERIZER_DESC Desc =
    { D3D12_FILL_MODE_SOLID, //FillMode
    D3D12_CULL_MODE_BACK, //CullMode
    0x0, //FrontCounterClockwise
    0x0/*0.000000f*/, //DepthBias
    0.f, //DepthBiasClamp
    0.f, //SlopeScaledDepthBias
    0x1, //DepthClipEnable
    0x0, //ScissorEnable
    0x0, //MultisampleEnable
    0x0  //AntialiasedLineEnable
    };

    m_pRasterizerState_Fullscreen_NoScissor = Desc;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::States::CreateSamplerStates()
{
    //
    // Create SamplerState_PointClamp
    //

    D3D12_SAMPLER_DESC SamplerDesc;
    memset(&SamplerDesc, 0, sizeof(SamplerDesc));
    SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;;
    SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    SamplerDesc.MipLODBias = 0.0f;
    SamplerDesc.MaxAnisotropy = 1;
    SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    SamplerDesc.BorderColor[0] = -D3D12_FLOAT32_MAX;
    SamplerDesc.MinLOD = 0;
    SamplerDesc.MaxLOD = 0;

    m_pSamplerState_PointClamp = SamplerDesc;

    //
    // Create SamplerState_PointBorder
    //

    SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;

    m_pSamplerState_PointBorder = SamplerDesc;

    //
    // Create SamplerState_LinearClamp
    //

    SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    
    m_pSamplerState_LinearClamp = SamplerDesc;

#if ENABLE_DEBUG_MODES
    //
    // Create SamplerState_PointWrap
    //

    SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

    m_pSamplerState_PointWrap = SamplerDesc;
#endif
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::States::Init()
{
    CreateBlendStates();
    CreateDepthStencilStates();
    CreateRasterizerStates();
    CreateSamplerStates();
}

#endif // SUPPORT_D3D12
