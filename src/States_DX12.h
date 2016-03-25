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

#pragma once
#include "Common.h"

namespace GFSDK
{
namespace SSAO
{
namespace D3D12
{

//--------------------------------------------------------------------------------
class States
{
public:
    States()
    {
    }

    void Create(GFSDK_D3D12_GraphicsContext* pD3DDevice);
    void Release();

    D3D12_BLEND_DESC* GetBlendStateMultiplyPreserveAlpha()
    {
        return &m_pBlendState_Multiply_PreserveAlpha;
    }
    D3D12_BLEND_DESC* GetBlendStateDisabledPreserveAlpha()
    {
        return &m_pBlendState_Disabled_PreserveAlpha;
    }
    D3D12_BLEND_DESC* GetBlendStateDisabled()
    {
        return &m_pBlendState_Disabled;
    }
    D3D12_DEPTH_STENCIL_DESC* GetDepthStencilStateDisabled()
    {
        return &m_pDepthStencilState_Disabled;
    }
    D3D12_RASTERIZER_DESC* GetRasterizerStateFullscreenNoScissor()
    {
        return &m_pRasterizerState_Fullscreen_NoScissor;
    }
    D3D12_SAMPLER_DESC& GetSamplerStatePointClamp()
    {
        return m_pSamplerState_PointClamp;
    }
    D3D12_SAMPLER_DESC& GetSamplerStatePointBorder()
    {
        return m_pSamplerState_PointBorder;
    }
    D3D12_SAMPLER_DESC& GetSamplerStateLinearClamp()
    {
        return m_pSamplerState_LinearClamp;
    }
#if ENABLE_DEBUG_MODES
    D3D12_SAMPLER_DESC& GetSamplerStatePointWrap()
    {
        return m_pSamplerState_PointWrap;
    }
#endif

private:
    void CreateBlendStates();
    void CreateDepthStencilStates();
    void CreateRasterizerStates();
    void CreateSamplerStates();

    D3D12_BLEND_DESC m_pBlendState_Disabled;
    D3D12_BLEND_DESC m_pBlendState_Multiply_PreserveAlpha;
    D3D12_BLEND_DESC m_pBlendState_Disabled_PreserveAlpha;
    D3D12_DEPTH_STENCIL_DESC m_pDepthStencilState_Disabled;
    D3D12_RASTERIZER_DESC m_pRasterizerState_Fullscreen_NoScissor;
    D3D12_SAMPLER_DESC m_pSamplerState_PointClamp;
    D3D12_SAMPLER_DESC m_pSamplerState_PointBorder;
    D3D12_SAMPLER_DESC m_pSamplerState_LinearClamp;
#if ENABLE_DEBUG_MODES
    D3D12_SAMPLER_DESC m_pSamplerState_PointWrap;
#endif
};

} // namespace D3D12
} // namespace SSAO
} // namespace GFSDK
#endif // SUPPORT_D3D12
