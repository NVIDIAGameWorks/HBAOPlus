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

#pragma once
#include "Common.h"

namespace GFSDK
{
namespace SSAO
{
namespace D3D11
{

//--------------------------------------------------------------------------------
struct VertexShader
{
    VertexShader() : VS(NULL)
    {
    }

    void Create(ID3D11Device* pD3DDevice, const void *pShaderBytecode, SIZE_T BytecodeLength)
    {
        THROW_IF_FAILED(pD3DDevice->CreateVertexShader(pShaderBytecode, BytecodeLength, NULL, &VS));
    }

    void Release(ID3D11Device*)
    {
        SAFE_RELEASE(VS);
    }

    operator ID3D11VertexShader*() const
    {
        return VS;
    }

private:
    ID3D11VertexShader* VS;
};

//--------------------------------------------------------------------------------
struct GeometryShader
{
    GeometryShader() : GS(NULL)
    {
    }

    void Create(ID3D11Device* pD3DDevice, const void *pShaderBytecode, SIZE_T BytecodeLength)
    {
        THROW_IF_FAILED(pD3DDevice->CreateGeometryShader(pShaderBytecode, BytecodeLength, NULL, &GS));
    }

    void Release(ID3D11Device*)
    {
        SAFE_RELEASE(GS);
    }

    operator ID3D11GeometryShader*() const
    {
        return GS;
    }

private:
    ID3D11GeometryShader* GS;
};

//--------------------------------------------------------------------------------
struct PixelShader
{
    PixelShader() : PS(NULL)
    {
    }

    void Create(ID3D11Device* pD3DDevice, const void *pShaderBytecode, SIZE_T BytecodeLength)
    {
        THROW_IF_FAILED(pD3DDevice->CreatePixelShader(pShaderBytecode, BytecodeLength, NULL, &PS));
    }

    void Release(ID3D11Device*)
    {
        SAFE_RELEASE(PS);
    }

    operator ID3D11PixelShader*() const
    {
        return PS;
    }

private:
    ID3D11PixelShader* PS;
};

//--------------------------------------------------------------------------------
struct DevicePointer
{
    DevicePointer(ID3D11Device* pDeviceIn) : pDevice(pDeviceIn)
    {
    }

    DevicePointer() : pDevice(NULL)
    {
    }

    operator ID3D11Device*() const
    {
        return pDevice;
    }

private:
    ID3D11Device* pDevice;
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
    void Create(ID3D11Device* pDevice)
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

#include "shaders/out/D3D11/DebugAO_PS.h"

struct DebugShaders
{
    void Create(ID3D11Device* pDevice)
    {
        DebugAO_PS.Create(DevicePointer(pDevice));
    }
    void Release()
    {
        DebugAO_PS.Release(DevicePointer());
    }

    Generated::DebugAO_PS DebugAO_PS;
};

#endif //ENABLE_DEBUG_MODES

} // namespace D3D11
} // namespace SSAO
} // namespace GFSDK

#endif // SUPPORT_D3D11
