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
        Init();
    }

    void Init();

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
