/*
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#pragma once
#include "Viewports.h"
#include "InputDepthInfo.h"
#include "MathUtil.h"
#include "RandomTexture.h"
#include "shaders/src/ConstantBuffers.hlsl"

namespace GFSDK
{
namespace SSAO
{

//--------------------------------------------------------------------------------
class GlobalConstants
{
public:
    GlobalConstants()
    {
        ZERO_STRUCT(m_Data);

        // Can be useful for figuring out the HBAO+ version from an APIC or NSight capture
        m_Data.u4BuildVersion = GFSDK_SSAO_Version();
    }

    void SetAOParameters(const GFSDK_SSAO_Parameters& Params, const InputDepthInfo& InputDepth);
    void SetRenderMask(GFSDK_SSAO_RenderMask RenderMask);
    void SetDepthData(const InputDepthInfo& InputDepth);
    void SetResolutionConstants(const SSAO::Viewports &Viewports);
    void SetNormalData(const GFSDK_SSAO_InputNormalData& NormalData);

protected:
    GlobalConstantBuffer m_Data;

    void SetDepthLinearizationConstants(const InputDepthInfo& InputDepth);
    void SetProjectionConstants(const InputDepthInfo& InputDepth);
    void SetViewportConstants(const InputDepthInfo& InputDepth);

    void SetBlurConstants(const GFSDK_SSAO_BlurParameters& Params, const InputDepthInfo& InputDepth);
    void SetAORadiusConstants(const GFSDK_SSAO_Parameters& Params, const InputDepthInfo& InputDepth);
    void SetDepthThresholdConstants(const GFSDK_SSAO_DepthThreshold& Params);
};

//--------------------------------------------------------------------------------
class PerPassConstants
{
public:
    PerPassConstants()
    {
        ZERO_STRUCT(m_Data);
    }

    void SetOffset(UINT OffsetX, UINT OffsetY)
    {
        m_Data.PerPassConstants.f2Offset.X = float(OffsetX) + 0.5f;
        m_Data.PerPassConstants.f2Offset.Y = float(OffsetY) + 0.5f;
    }

    void SetJitter(float4 Jitter)
    {
        m_Data.PerPassConstants.f4Jitter = Jitter;
    }

    void SetSliceIndex(UINT SliceIndex)
    {
        m_Data.PerPassConstants.fSliceIndex = float(SliceIndex);
        m_Data.PerPassConstants.uSliceIndex = SliceIndex;
    }

protected:
    PerPassConstantBuffer m_Data;
};

//--------------------------------------------------------------------------------
#if SUPPORT_D3D11
namespace D3D11
{

class BaseConstantBuffer
{
public:
    BaseConstantBuffer(UINT ByteWidth)
        : m_ByteWidth(ByteWidth)
        , m_pConstantBuffer(NULL)
    {
    }

    void Create(ID3D11Device* pD3DDevice, D3D11_SUBRESOURCE_DATA* pSubresourceData = NULL)
    {
        D3D11_BUFFER_DESC BufferDesc = 
        {
             m_ByteWidth, //ByteWidth
             D3D11_USAGE_DEFAULT, //Usage
             D3D11_BIND_CONSTANT_BUFFER, //BindFlags
             0, //CPUAccessFlags
             0  //MiscFlags
        };

        // The D3D11 runtime requires constant buffer sizes to be multiple of 16 bytes
        ASSERT(BufferDesc.ByteWidth % 16 == 0);

        ASSERT(!m_pConstantBuffer);
        SAFE_D3D_CALL( pD3DDevice->CreateBuffer(&BufferDesc, pSubresourceData, &m_pConstantBuffer) );
    }

    void Release()
    {
        SAFE_RELEASE(m_pConstantBuffer);
    }

    void UpdateCB(ID3D11DeviceContext* pDeviceContext, void* pData)
    {
        ASSERT(m_pConstantBuffer);

        pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, pData, 0, 0);
    }

    ID3D11Buffer*& GetCB()
    {
        return m_pConstantBuffer;
    }

protected:
    UINT m_ByteWidth;
    ID3D11Buffer *m_pConstantBuffer;
};

//--------------------------------------------------------------------------------
class GlobalConstantBuffer : public GlobalConstants, public D3D11::BaseConstantBuffer
{
public:
    GlobalConstantBuffer()
        : GlobalConstants()
        , D3D11::BaseConstantBuffer(sizeof(m_Data))
    {
    }
    void UpdateBuffer(ID3D11DeviceContext* pDeviceContext, GFSDK_SSAO_RenderMask RenderMask)
    {
        SetRenderMask(RenderMask);
        UpdateCB(pDeviceContext, &m_Data);
    }
};

//--------------------------------------------------------------------------------
class PerPassConstantBuffer : public PerPassConstants, public D3D11::BaseConstantBuffer
{
public:
    PerPassConstantBuffer() 
        : PerPassConstants()
        , D3D11::BaseConstantBuffer(sizeof(m_Data))
    {
    }
    void Create(ID3D11Device* pD3DDevice)
    {
        D3D11_SUBRESOURCE_DATA SubresourceData = { &m_Data };
        D3D11::BaseConstantBuffer::Create(pD3DDevice, &SubresourceData);
    }
};

//--------------------------------------------------------------------------------
class PerPassConstantBuffers
{
public:
    void Create(ID3D11Device* pD3DDevice)
    {
        for (UINT SliceIndex = 0; SliceIndex < SIZEOF_ARRAY(m_CBs); ++SliceIndex)
        {
            m_CBs[SliceIndex].SetOffset(SliceIndex % 4, SliceIndex / 4);
            m_CBs[SliceIndex].SetJitter(m_RandomTexture.GetJitter(SliceIndex));
            m_CBs[SliceIndex].SetSliceIndex(SliceIndex);
            m_CBs[SliceIndex].Create(pD3DDevice);
        }
    }
    void Release()
    {
        for (UINT PassIndex = 0; PassIndex < SIZEOF_ARRAY(m_CBs); ++PassIndex)
        {
            m_CBs[PassIndex].Release();
        }
    }
    ID3D11Buffer*& GetCB(UINT PassIndex)
    {
        ASSERT(PassIndex < SIZEOF_ARRAY(m_CBs));
        return m_CBs[PassIndex].GetCB();
    }

private:
    PerPassConstantBuffer m_CBs[16];
    GFSDK::SSAO::RandomTexture m_RandomTexture;
};

} // namespace D3D11
#endif // SUPPORT_D3D11

//--------------------------------------------------------------------------------
#if SUPPORT_D3D12
namespace D3D12
{

//--------------------------------------------------------------------------------
struct GFSDK_D3D12_ConstantBufferView
{
    ID3D12Resource* pResource;
    D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;
    D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
};

//--------------------------------------------------------------------------------
class BaseConstantBuffer
{
public:
    BaseConstantBuffer(UINT ByteWidth)
        : m_ByteWidth(ByteWidth)
    {
    }

    void Create(GFSDK_D3D12_GraphicsContext* pContext, CBVSRVUAVLayoutBase Base, UINT BaseOffset)
    {
        ASSERT(!m_pConstantBuffer.pResource);

        // CB size is required to be 256-byte aligned.
        const UINT AlignedByteWidth = ALIGNED_SIZE(m_ByteWidth, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

        THROW_IF_FAILED(pContext->pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD, pContext->NodeMask, pContext->NodeMask),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(AlignedByteWidth),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_pConstantBuffer.pResource)));

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_pConstantBuffer.pResource->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = AlignedByteWidth;

        m_pConstantBuffer.CpuHandle = pContext->DescHeaps.CBVSRVUAV.GetCPUHandle((UINT)Base + BaseOffset);
        pContext->pDevice->CreateConstantBufferView(&cbvDesc, m_pConstantBuffer.CpuHandle);

        m_pConstantBuffer.pResource->Map(0, nullptr, reinterpret_cast<void**>(&mCBUploadPtr));
    }

    void Release()
    {
        if (m_pConstantBuffer.pResource)
        {
            m_pConstantBuffer.pResource->Unmap(0, nullptr);
            SAFE_RELEASE(m_pConstantBuffer.pResource);
        }
    }

    void UpdateCB(GFSDK_D3D12_GraphicsContext* pGraphicsContext, void* pData)
    {
        memcpy(mCBUploadPtr, pData, m_ByteWidth);
    }

    GFSDK_D3D12_ConstantBufferView m_pConstantBuffer;

protected:
    UINT m_ByteWidth;
    void* mCBUploadPtr;
};

//--------------------------------------------------------------------------------
class GlobalConstantBuffer : public GlobalConstants, public D3D12::BaseConstantBuffer
{
public:
    GlobalConstantBuffer()
        : GlobalConstants()
        , D3D12::BaseConstantBuffer(sizeof(m_Data))
    {
    }
    void UpdateBuffer(GFSDK_D3D12_GraphicsContext* pGraphicsContext, GFSDK_SSAO_RenderMask RenderMask)
    {
        SetRenderMask(RenderMask);
        UpdateCB(pGraphicsContext, &m_Data);
    }
};

//--------------------------------------------------------------------------------
class PerPassConstantBuffer : public PerPassConstants, public D3D12::BaseConstantBuffer
{
public:
    PerPassConstantBuffer()
        : PerPassConstants()
        , D3D12::BaseConstantBuffer(sizeof(m_Data))
    {
    }
    void UpdateBuffer(GFSDK_D3D12_GraphicsContext* pGraphicsContext)
    {
        UpdateCB(pGraphicsContext, &m_Data);
    }
};

//--------------------------------------------------------------------------------
class PerPassConstantBuffers
{
public:
    void Create(GFSDK_D3D12_GraphicsContext* pD3DDevice, CBVSRVUAVLayoutBase Base)
    {
        for (UINT SliceIndex = 0; SliceIndex < SIZEOF_ARRAY(m_CBs); ++SliceIndex)
        {
            m_CBs[SliceIndex].SetOffset(SliceIndex % 4, SliceIndex / 4);
            m_CBs[SliceIndex].SetJitter(m_RandomTexture.GetJitter(SliceIndex));
            m_CBs[SliceIndex].SetSliceIndex(SliceIndex);

            m_CBs[SliceIndex].Create(pD3DDevice, Base, SliceIndex);
            m_CBs[SliceIndex].UpdateBuffer(pD3DDevice);
        }
    }
    void Release()
    {
        for (UINT PassIndex = 0; PassIndex < SIZEOF_ARRAY(m_CBs); ++PassIndex)
        {
            m_CBs[PassIndex].Release();
        }
    }

private:
    PerPassConstantBuffer m_CBs[16];
    GFSDK::SSAO::RandomTexture m_RandomTexture;
};

} // namespace D3D12
#endif // SUPPORT_D3D12

#if SUPPORT_GL
namespace GL
{

class BaseConstantBuffer
{
public:
    BaseConstantBuffer(UINT ByteWidth, GLuint BindingPoint)
        : m_ByteWidth(ByteWidth)
        , m_BufferId(0)
        , m_BindingPoint(BindingPoint)
    {
    }

    void Create(const GFSDK_SSAO_GLFunctions& GL)
    {
        ASSERT(!m_BufferId);
        GL.glGenBuffers(1, &m_BufferId);

        GL.glBindBuffer(GL_UNIFORM_BUFFER, m_BufferId);
        GL.glBufferData(GL_UNIFORM_BUFFER, m_ByteWidth, NULL, GL_DYNAMIC_DRAW);
        GL.glBindBuffer(GL_UNIFORM_BUFFER, 0);
        ASSERT_GL_ERROR(GL);
    }

    void Release(const GFSDK_SSAO_GLFunctions& GL)
    {
        GL.glDeleteBuffers(1, &m_BufferId);
        m_BufferId = 0;
    }

    void Unbind(const GFSDK_SSAO_GLFunctions& GL)
    {
        GL.glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, 0);
        ASSERT_GL_ERROR(GL);
    }

    void UpdateCB(const GFSDK_SSAO_GLFunctions& GL, void* pData)
    {
        ASSERT(m_BufferId);

        GL.glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_BufferId);
        ASSERT_GL_ERROR(GL);

        // Do not use glMapBuffer for updating constant buffers (slow path on GL).
        // glBufferSubData has a fast path for UBO.
        GL.glBufferSubData(GL_UNIFORM_BUFFER, 0, m_ByteWidth, pData);
        ASSERT_GL_ERROR(GL);
    }

    GLuint GetBindingPoint()
    {
        return m_BindingPoint;
    }

    GLuint GetBufferId()
    {
        return m_BufferId;
    }

    enum BindingPoints
    {
        BINDING_POINT_GLOBAL_UBO = 0,
        BINDING_POINT_NORMAL_UBO = 1,
        BINDING_POINT_PER_PASS_UBO = 2,
    };

protected:
    UINT m_ByteWidth;
    GLuint m_BufferId;
    GLuint m_BindingPoint;
};

//--------------------------------------------------------------------------------
class GlobalConstantBuffer : public GlobalConstants, public GL::BaseConstantBuffer
{
public:
    GlobalConstantBuffer() 
        : GlobalConstants()
        , GL::BaseConstantBuffer(sizeof(m_Data), BINDING_POINT_GLOBAL_UBO)
    {
    }
    void UpdateBuffer(const GFSDK_SSAO_GLFunctions& GL, GFSDK_SSAO_RenderMask RenderMask)
    {
        SetRenderMask(RenderMask);
        UpdateCB(GL, &m_Data);
    }
};

//--------------------------------------------------------------------------------
class PerPassConstantBuffer : public PerPassConstants, public GL::BaseConstantBuffer
{
public:
    PerPassConstantBuffer() 
        : PerPassConstants()
        , GL::BaseConstantBuffer(sizeof(m_Data), BINDING_POINT_PER_PASS_UBO)
    {
    }
    void UpdateBuffer(const GFSDK_SSAO_GLFunctions& GL)
    {
        UpdateCB(GL, &m_Data);
    }
};

//--------------------------------------------------------------------------------
class PerPassConstantBuffers
{
public:
    void Create(const GFSDK_SSAO_GLFunctions& GL)
    {
        for (UINT SliceIndex = 0; SliceIndex < SIZEOF_ARRAY(m_CBs); ++SliceIndex)
        {
            m_CBs[SliceIndex].Create(GL);

            UINT JitterX = SliceIndex % 4;
            UINT JitterY = SliceIndex / 4;

            m_CBs[SliceIndex].SetOffset(JitterX, JitterY);
            m_CBs[SliceIndex].SetJitter(GetJitterVector(JitterX, JitterY));
            m_CBs[SliceIndex].SetSliceIndex(SliceIndex);
            m_CBs[SliceIndex].UpdateBuffer(GL);
        }
    }
    void Release(const GFSDK_SSAO_GLFunctions& GL)
    {
        for (UINT PassIndex = 0; PassIndex < SIZEOF_ARRAY(m_CBs); ++PassIndex)
        {
            m_CBs[PassIndex].Release(GL);
        }
    }
    GLuint GetBufferId(UINT PassIndex)
    {
        ASSERT(PassIndex < SIZEOF_ARRAY(m_CBs));
        return m_CBs[PassIndex].GetBufferId();
    }
    GLuint GetBindingPoint()
    {
        return m_CBs[0].GetBindingPoint();
    }
    void Unbind(const GFSDK_SSAO_GLFunctions& GL)
    {
        m_CBs[0].Unbind(GL);
    }

private:
    float4 GetJitterVector(UINT JitterX, UINT JitterY)
    {
        // To match the reference D3D11 implementation
        JitterY = 3 - JitterY;
        return m_RandomTexture.GetJitter(JitterY * 4 + JitterX);
    }

    PerPassConstantBuffer m_CBs[16];
    GFSDK::SSAO::RandomTexture m_RandomTexture;
};

} // namespace GL
#endif // SUPPORT_GL

} // namespace SSAO
} // namespace GFSDK
