/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#if SUPPORT_D3D11

#include "States_DX11.h"

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::States::CreateBlendStates(ID3D11Device* pD3DDevice)
{
    //
    // Create BlendState_Disabled
    //

    D3D11_BLEND_DESC BlendStateDesc;
    BlendStateDesc.AlphaToCoverageEnable = FALSE;
    BlendStateDesc.IndependentBlendEnable = TRUE;

    for (UINT i = 0; i < SIZEOF_ARRAY(BlendStateDesc.RenderTarget); ++i)
    {
        BlendStateDesc.RenderTarget[i].BlendEnable = FALSE;
        BlendStateDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }

    SAFE_D3D_CALL( pD3DDevice->CreateBlendState(&BlendStateDesc, &m_pBlendState_Disabled) );

    //
    // Create BlendState_Multiply_PreserveAlpha
    //

    BlendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
    BlendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
    BlendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
    BlendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    BlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    BlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

    SAFE_D3D_CALL( pD3DDevice->CreateBlendState(&BlendStateDesc, &m_pBlendState_Multiply_PreserveAlpha) );

    //
    // Create BlendState_Disabled_PreserveAlpha
    //

    BlendStateDesc.RenderTarget[0].BlendEnable = FALSE;

    SAFE_D3D_CALL( pD3DDevice->CreateBlendState(&BlendStateDesc, &m_pBlendState_Disabled_PreserveAlpha) );
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::States::CreateDepthStencilStates(ID3D11Device* pD3DDevice)
{
    //
    // Create DepthStencilState_Disabled
    //

    static D3D11_DEPTH_STENCIL_DESC DepthStencilDesc = 
    {0x0, //DepthEnable
     D3D11_DEPTH_WRITE_MASK_ZERO, //DepthWriteMask
     D3D11_COMPARISON_NEVER, //DepthFunc
     0x0, //StencilEnable
     0xFF, //StencilReadMask
     0xFF, //StencilWriteMask
     
    {D3D11_STENCIL_OP_KEEP, //StencilFailOp
     D3D11_STENCIL_OP_KEEP, //StencilDepthFailOp
     D3D11_STENCIL_OP_KEEP, //StencilPassOp
     D3D11_COMPARISON_ALWAYS  //StencilFunc
    }, //FrontFace
     
    {D3D11_STENCIL_OP_KEEP, //StencilFailOp
     D3D11_STENCIL_OP_KEEP, //StencilDepthFailOp
     D3D11_STENCIL_OP_KEEP, //StencilPassOp
     D3D11_COMPARISON_ALWAYS  //StencilFunc
    }  //BackFace
    };

    SAFE_D3D_CALL( pD3DDevice->CreateDepthStencilState(&DepthStencilDesc, &m_pDepthStencilState_Disabled) );
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::States::CreateRasterizerStates(ID3D11Device* pD3DDevice)
{
    //
    // Create RasterizerState_Fullscreen_NoScissor
    //

    static D3D11_RASTERIZER_DESC D3D11_RASTERIZER_DESC_0 = 
    {D3D11_FILL_SOLID, //FillMode
     D3D11_CULL_BACK, //CullMode
     0x0, //FrontCounterClockwise
     0x0/*0.000000f*/, //DepthBias
                   0.f, //DepthBiasClamp
                   0.f, //SlopeScaledDepthBias
     0x1, //DepthClipEnable
     0x0, //ScissorEnable
     0x0, //MultisampleEnable
     0x0  //AntialiasedLineEnable
    };

    SAFE_D3D_CALL( pD3DDevice->CreateRasterizerState(&D3D11_RASTERIZER_DESC_0, &m_pRasterizerState_Fullscreen_NoScissor) );
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::States::CreateSamplerStates(ID3D11Device* pD3DDevice)
{
    //
    // Create SamplerState_PointClamp
    //

    D3D11_SAMPLER_DESC SamplerDesc;
    memset(&SamplerDesc, 0, sizeof(SamplerDesc));
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.MipLODBias = 0.0f;
    SamplerDesc.MaxAnisotropy = 1;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SamplerDesc.BorderColor[0] = -D3D11_FLOAT32_MAX;
    SamplerDesc.MinLOD = 0;
    SamplerDesc.MaxLOD = 0;

    SAFE_D3D_CALL( pD3DDevice->CreateSamplerState(&SamplerDesc, &m_pSamplerState_PointClamp) );

    //
    // Create SamplerState_PointBorder
    //

    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;

    SAFE_D3D_CALL( pD3DDevice->CreateSamplerState(&SamplerDesc, &m_pSamplerState_PointBorder) );

    //
    // Create SamplerState_LinearClamp
    //

    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;

    SAFE_D3D_CALL( pD3DDevice->CreateSamplerState(&SamplerDesc, &m_pSamplerState_LinearClamp) );

#if ENABLE_DEBUG_MODES
    //
    // Create SamplerState_PointWrap
    //

    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;

    SAFE_D3D_CALL( pD3DDevice->CreateSamplerState(&SamplerDesc, &m_pSamplerState_PointWrap) );
#endif
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::States::Create(ID3D11Device* pD3DDevice)
{
    CreateBlendStates(pD3DDevice);
    CreateDepthStencilStates(pD3DDevice);
    CreateRasterizerStates(pD3DDevice);
    CreateSamplerStates(pD3DDevice);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::States::Release()
{
    SAFE_RELEASE(m_pBlendState_Disabled);
    SAFE_RELEASE(m_pBlendState_Multiply_PreserveAlpha);
    SAFE_RELEASE(m_pBlendState_Disabled_PreserveAlpha);
    SAFE_RELEASE(m_pDepthStencilState_Disabled);
    SAFE_RELEASE(m_pRasterizerState_Fullscreen_NoScissor);
    SAFE_RELEASE(m_pSamplerState_PointClamp);
    SAFE_RELEASE(m_pSamplerState_PointBorder);
    SAFE_RELEASE(m_pSamplerState_LinearClamp);
#if ENABLE_DEBUG_MODES
    SAFE_RELEASE(m_pSamplerState_PointWrap);
#endif
}

#endif // SUPPORT_D3D11
