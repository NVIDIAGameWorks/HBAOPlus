/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

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
    //SAFE_D3D_CALL(pD3DDevice->CreateBlendState(&BlendStateDesc, &m_pBlendState_Disabled));

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
    //SAFE_D3D_CALL(pD3DDevice->CreateBlendState(&BlendStateDesc, &m_pBlendState_Multiply_PreserveAlpha));

    //
    // Create BlendState_Disabled_PreserveAlpha
    //

    BlendStateDesc.RenderTarget[0].BlendEnable = FALSE;

    m_pBlendState_Disabled_PreserveAlpha = BlendStateDesc;
    //SAFE_D3D_CALL(pD3DDevice->CreateBlendState(&BlendStateDesc, &m_pBlendState_Disabled_PreserveAlpha));
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
    //SAFE_D3D_CALL(pD3DDevice->CreateDepthStencilState(&DepthStencilDesc, &m_pDepthStencilState_Disabled));
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::States::CreateRasterizerStates()
{
    //
    // Create RasterizerState_Fullscreen_NoScissor
    //

    static D3D12_RASTERIZER_DESC D3D12_RASTERIZER_DESC_0 =
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

    m_pRasterizerState_Fullscreen_NoScissor = D3D12_RASTERIZER_DESC_0;
    //SAFE_D3D_CALL(pD3DDevice->CreateRasterizerState(&D3D12_RASTERIZER_DESC_0, &m_pRasterizerState_Fullscreen_NoScissor));
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
    //SAFE_D3D_CALL(pD3DDevice->CreateSamplerState(&SamplerDesc, &m_pSamplerState_PointClamp));

    //
    // Create SamplerState_PointBorder
    //

    SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;

    m_pSamplerState_PointBorder = SamplerDesc;
    //SAFE_D3D_CALL(pD3DDevice->CreateSamplerState(&SamplerDesc, &m_pSamplerState_PointBorder));

    //
    // Create SamplerState_LinearClamp
    //

    SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    
    m_pSamplerState_LinearClamp = SamplerDesc;
    //SAFE_D3D_CALL(pD3DDevice->CreateSamplerState(&SamplerDesc, &m_pSamplerState_LinearClamp));

#if ENABLE_DEBUG_MODES
    //
    // Create SamplerState_PointWrap
    //

    SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

    m_pSamplerState_PointWrap = SamplerDesc;
    //SAFE_D3D_CALL(pD3DDevice->CreateSamplerState(&SamplerDesc, &m_pSamplerState_PointWrap));
#endif
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::States::Create(GFSDK_D3D12_GraphicsContext* pD3DDevice)
{
    CreateBlendStates();
    CreateDepthStencilStates();
    CreateRasterizerStates();
    CreateSamplerStates();
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::States::Release()
{
    //SAFE_RELEASE(m_pBlendState_Disabled);
    //SAFE_RELEASE(m_pBlendState_Multiply_PreserveAlpha);
    //SAFE_RELEASE(m_pBlendState_Disabled_PreserveAlpha);
    //SAFE_RELEASE(m_pDepthStencilState_Disabled);
    //SAFE_RELEASE(m_pRasterizerState_Fullscreen_NoScissor);
    //SAFE_RELEASE(m_pSamplerState_PointClamp);
    //SAFE_RELEASE(m_pSamplerState_PointBorder);
    //SAFE_RELEASE(m_pSamplerState_LinearClamp);
#if ENABLE_DEBUG_MODES
    //SAFE_RELEASE(m_pSamplerState_PointWrap);
#endif
}

#endif // SUPPORT_D3D12
