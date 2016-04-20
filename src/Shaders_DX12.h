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
struct VertexShader
{
    VertexShader()
    {
        VS.BytecodeLength = 0;
        VS.pShaderBytecode = nullptr;
    }

    void Create(ID3D12Device* pD3DDevice, const void *pShaderBytecode, SIZE_T BytecodeLength)
    {
        VS.pShaderBytecode = pShaderBytecode;
        VS.BytecodeLength = BytecodeLength;
    }

    void Release(ID3D12Device*)
    {
        VS.pShaderBytecode = nullptr;
        VS.BytecodeLength = 0;
    }

    operator GFSDK_D3D12_VertexShader*()
    {
        return &VS;
    }

private:
    GFSDK_D3D12_VertexShader VS;
};

//--------------------------------------------------------------------------------
struct GeometryShader
{
    GeometryShader()
    {
        GS.BytecodeLength = 0;
        GS.pShaderBytecode = nullptr;
    }

    void Create(ID3D12Device* pD3DDevice, const void *pShaderBytecode, SIZE_T BytecodeLength)
    {
        GS.pShaderBytecode = pShaderBytecode;
        GS.BytecodeLength = BytecodeLength;
    }

    void Release(ID3D12Device*)
    {
        GS.pShaderBytecode = nullptr;
        GS.BytecodeLength = 0;
    }

    operator GFSDK_D3D12_GeometryShader*()
    {
        return &GS;
    }

private:
    GFSDK_D3D12_GeometryShader GS;
};

//--------------------------------------------------------------------------------
struct PixelShader
{
    PixelShader()
    {
        PS.BytecodeLength = 0;
        PS.pShaderBytecode = nullptr;
    }

    void Create(ID3D12Device* pD3DDevice, const void *pShaderBytecode, SIZE_T BytecodeLength)
    {
        PS.BytecodeLength = BytecodeLength;
        PS.pShaderBytecode = pShaderBytecode;
    }

    void Release(ID3D12Device*)
    {
        PS.pShaderBytecode = nullptr;
        PS.BytecodeLength = 0;
    }

    operator GFSDK_D3D12_PixelShader*()
    {
        return &PS;
    }

private:
    GFSDK_D3D12_PixelShader PS;
};

//--------------------------------------------------------------------------------
struct DevicePointer
{
    DevicePointer(ID3D12Device* pDeviceIn) : pDevice(pDeviceIn)
    {
    }

    DevicePointer() : pDevice(NULL)
    {
    }

    operator ID3D12Device*() const
    {
        return pDevice;
    }

private:
    ID3D12Device* pDevice;
};

//--------------------------------------------------------------------------------
#include "shaders/out/D3D11/FullScreenTriangle_VS.h"
#include "shaders/out/D3D11/CopyDepth_PS.h"
#include "shaders/out/D3D11/LinearizeDepth_PS.h"
#include "shaders/out/D3D11/DeinterleaveDepth_PS.h"
#include "shaders/out/D3D11/DebugNormals_PS.h"
#include "shaders/out/D3D11/ReconstructNormal_PS.h"
#include "shaders/out/D3D11/ReinterleaveAO_PS.h"
#include "shaders/out/D3D11/BlurX_PS.h"
#include "shaders/out/D3D11/BlurY_PS.h"
#include "shaders/out/D3D11/CoarseAO_PS.h"
#include "shaders/out/D3D11/CoarseAO_GS.h"

//--------------------------------------------------------------------------------
struct Shaders
{
    void Create(ID3D12Device* pDevice)
    {
        FullScreenTriangle_VS.Create(DevicePointer(pDevice));
        CopyDepth_PS.Create(DevicePointer(pDevice));
        LinearizeDepth_PS.Create(DevicePointer(pDevice));
        DeinterleaveDepth_PS.Create(DevicePointer(pDevice));
        DebugNormals_PS.Create(DevicePointer(pDevice));
        ReconstructNormal_PS.Create(DevicePointer(pDevice));
        ReinterleaveAO_PS.Create(DevicePointer(pDevice));
        BlurX_PS.Create(DevicePointer(pDevice));
        BlurY_PS.Create(DevicePointer(pDevice));
        CoarseAO_PS.Create(DevicePointer(pDevice));
        CoarseAO_GS.Create(DevicePointer(pDevice));
    }

    void Release()
    {
        FullScreenTriangle_VS.Release(DevicePointer());
        CopyDepth_PS.Release(DevicePointer());
        LinearizeDepth_PS.Release(DevicePointer());
        DeinterleaveDepth_PS.Release(DevicePointer());
        DebugNormals_PS.Release(DevicePointer());
        ReconstructNormal_PS.Release(DevicePointer());
        ReinterleaveAO_PS.Release(DevicePointer());
        BlurX_PS.Release(DevicePointer());
        BlurY_PS.Release(DevicePointer());
        CoarseAO_PS.Release(DevicePointer());
        CoarseAO_GS.Release(DevicePointer());
    }

    Generated::FullScreenTriangle_VS FullScreenTriangle_VS;
    Generated::CopyDepth_PS CopyDepth_PS;
    Generated::LinearizeDepth_PS LinearizeDepth_PS;
    Generated::DeinterleaveDepth_PS DeinterleaveDepth_PS;
    Generated::DebugNormals_PS DebugNormals_PS;
    Generated::ReconstructNormal_PS ReconstructNormal_PS;
    Generated::ReinterleaveAO_PS ReinterleaveAO_PS;
    Generated::BlurX_PS BlurX_PS;
    Generated::BlurY_PS BlurY_PS;
    Generated::CoarseAO_PS CoarseAO_PS;
    Generated::CoarseAO_GS CoarseAO_GS;
};

#if ENABLE_DEBUG_MODES

#include "shaders/out/D3D11/HBAO_PS.h"
#include "shaders/out/D3D11/DebugAO_PS.h"

struct DebugShaders
{
    void Create(ID3D12Device* pDevice)
    {
        HBAO_PS.Create(pDevice);
        DebugAO_PS.Create(pDevice);
    }
    void Release()
    {
        HBAO_PS.Release();
        DebugAO_PS.Release();
    }

    Generated::HBAO_PS HBAO_PS;
    Generated::DebugAO_PS DebugAO_PS;
};

#endif //ENABLE_DEBUG_MODES

} // namespace D3D12
} // namespace SSAO
} // namespace GFSDK

#endif // SUPPORT_D3D12
