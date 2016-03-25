// This code contains NVIDIA Confidential Information and is disclosed 
// under the Mutual Non-Disclosure Agreement. 
// 
// Notice 
// ALL NVIDIA DESIGN SPECIFICATIONS AND CODE ("MATERIALS") ARE PROVIDED "AS IS" NVIDIA MAKES 
// NO REPRESENTATIONS, WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO 
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ANY IMPLIED WARRANTIES OF NONINFRINGEMENT, 
// MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. 
// 
// NVIDIA Corporation assumes no responsibility for the consequences of use of such 
// information or for any infringement of patents or other rights of third parties that may 
// result from its use. No license is granted by implication or otherwise under any patent 
// or patent rights of NVIDIA Corporation. No third party distribution is allowed unless 
// expressly authorized by NVIDIA.  Details are subject to change without notice. 
// This code supersedes and replaces all information previously supplied. 
// NVIDIA Corporation products are not authorized for use as critical 
// components in life support devices or systems without express written approval of 
// NVIDIA Corporation. 
// 
// Copyright © 2008-2015 NVIDIA Corporation. All rights reserved.
//
// NVIDIA Corporation and its licensors retain all intellectual property and proprietary
// rights in and to this software and related documentation and any modifications thereto.
// Any use, reproduction, disclosure or distribution of this software and related
// documentation without an express license agreement from NVIDIA Corporation is
// strictly prohibited.
//

#pragma once
#include "GFSDK_SSAO.h"
#include <stdio.h>
#include <assert.h>

namespace GFSDK
{
namespace SSAO
{
namespace D3D11
{

class InputDumpWriter
{
public:
    static const UINT FILE_VERSION = 5;

    enum Status
    {
        STATUS_NULL_ARGUMENT,
        STATUS_UNSUPPORTED_TEXTURE_SAMPLE_COUNT,
        STATUS_UNSUPPORTED_TEXTURE_ARRAY_SIZE,
        STATUS_UNSUPPORTED_TEXTURE_MIP_COUNT,
        STATUS_FOPEN_FAILED,
        STATUS_FWRITE_FAILED,
        STATUS_CREATE_TEXTURE_FAILED,
        STATUS_MAP_TEXTURE_FAILED,
        STATUS_OK,
    };

    InputDumpWriter()
        : m_pFile(NULL)
        , m_pBaseTexture(NULL)
        , m_pStagingTexture(NULL)
    {
    }

    ~InputDumpWriter()
    {
        SafeCloseFile();
        SAFE_RELEASE(m_pBaseTexture);
        SAFE_RELEASE(m_pStagingTexture);
    }

    #define WRITE_STRUCT(S)\
    {\
        UINT StructSize = sizeof(S);\
        if (fwrite(&StructSize, sizeof(StructSize), 1, m_pFile) != 1)\
        {\
            return STATUS_FWRITE_FAILED;\
        }\
        if (fwrite(&S, sizeof(S), 1, m_pFile) != 1)\
        {\
            return STATUS_FWRITE_FAILED;\
        }\
    }

    Status Write(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pDeviceContext,
        const GFSDK_SSAO_InputData_D3D11* pInputData,
        const GFSDK_SSAO_Parameters* pParameters,
        const char* pFilename)
    {
        if (!pDevice || !pDeviceContext || !pInputData || !pParameters || !pFilename)
        {
            return STATUS_NULL_ARGUMENT;
        }

        SafeCloseFile();
        if (fopen_s(&m_pFile, pFilename, "wb") || !m_pFile)
        {
            return STATUS_FOPEN_FAILED;
        }

        const UINT FileVersion = FILE_VERSION;
        if (fwrite(&FileVersion, sizeof(FileVersion), 1, m_pFile) != 1)
        {
            return STATUS_FWRITE_FAILED;
        }

        const GFSDK_SSAO_Version BuildVersion;
        WRITE_STRUCT(BuildVersion);

        GFSDK_SSAO_InputDepthData InputDepthData = pInputData->DepthData;
        WRITE_STRUCT(InputDepthData);

        GFSDK_SSAO_InputNormalData InputNormalData = pInputData->NormalData;
        WRITE_STRUCT(InputNormalData);

        GFSDK_SSAO_Parameters Parameters = *pParameters;
        WRITE_STRUCT(Parameters);

        InputDumpWriter::Status Status = WriteTexture(pDevice, pDeviceContext, pInputData->DepthData.pFullResDepthTextureSRV);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        if (pInputData->NormalData.Enable)
        {
            Status = WriteTexture(pDevice, pDeviceContext, pInputData->NormalData.pFullResNormalTextureSRV);
            if (Status != GFSDK_SSAO_OK)
            {
                return Status;
            }
        }

        return STATUS_OK;
    }

private:
    void SafeCloseFile()
    {
        if (m_pFile)
        {
            fclose(m_pFile);
            m_pFile = NULL;
        }
    }

    static void GetTextureDesc(ID3D11ShaderResourceView* pSRV, D3D11_TEXTURE2D_DESC* pTextureDesc)
    {
        ID3D11Texture2D* pBaseTexture;
        pSRV->GetResource((ID3D11Resource**)&pBaseTexture);

        pBaseTexture->GetDesc(pTextureDesc);
        SAFE_RELEASE(pBaseTexture);
    }

    bool CreateStagingTexture(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pDeviceContext,
        ID3D11ShaderResourceView* pSRV)
    {
        SAFE_RELEASE(m_pBaseTexture);
        pSRV->GetResource((ID3D11Resource**)&m_pBaseTexture);

        D3D11_TEXTURE2D_DESC TextureDesc;
        m_pBaseTexture->GetDesc(&TextureDesc);

        assert(TextureDesc.SampleDesc.Count == 1);
        assert(TextureDesc.ArraySize == 1);
        assert(TextureDesc.MipLevels == 1);

        D3D11_TEXTURE2D_DESC StagingTextureDesc = TextureDesc;
        StagingTextureDesc.Usage              = D3D11_USAGE_STAGING;
        StagingTextureDesc.BindFlags          = 0;
        StagingTextureDesc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
        StagingTextureDesc.MiscFlags          = 0;

        SAFE_RELEASE(m_pStagingTexture);
        if (pDevice->CreateTexture2D(&StagingTextureDesc, NULL, &m_pStagingTexture) != S_OK)
        {
            return false;
        }

        pDeviceContext->CopyResource(m_pStagingTexture, m_pBaseTexture);

        return true;
    }

    Status WriteTexture(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pDeviceContext,
        ID3D11ShaderResourceView* pSRV)
    {
        if (!pSRV)
        {
            return STATUS_NULL_ARGUMENT;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc;
        pSRV->GetDesc(&SrvDesc);

        D3D11_TEXTURE2D_DESC TextureDesc;
        GetTextureDesc(pSRV, &TextureDesc);

        if (TextureDesc.SampleDesc.Count != 1)
        {
            return STATUS_UNSUPPORTED_TEXTURE_SAMPLE_COUNT;
        }

        if (TextureDesc.ArraySize != 1)
        {
            return STATUS_UNSUPPORTED_TEXTURE_ARRAY_SIZE;
        }

        if (TextureDesc.MipLevels != 1)
        {
            return STATUS_UNSUPPORTED_TEXTURE_MIP_COUNT;
        }

        if (!CreateStagingTexture(pDevice, pDeviceContext, pSRV))
        {
            return STATUS_CREATE_TEXTURE_FAILED;
        }

        D3D11_MAPPED_SUBRESOURCE LockedRect;
        if (pDeviceContext->Map(m_pStagingTexture, 0, D3D11_MAP_READ, 0, &LockedRect) != S_OK)
        {
            return STATUS_MAP_TEXTURE_FAILED;
        }

        if (fwrite(&TextureDesc, sizeof(TextureDesc), 1, m_pFile) != 1)
        {
            return STATUS_FWRITE_FAILED;
        }

        if (fwrite(&SrvDesc, sizeof(SrvDesc), 1, m_pFile) != 1)
        {
            return STATUS_FWRITE_FAILED;
        }

        if (fwrite(&LockedRect.RowPitch, sizeof(LockedRect.RowPitch), 1, m_pFile) != 1)
        {
            return STATUS_FWRITE_FAILED;
        }

        if (fwrite(LockedRect.pData, TextureDesc.Height * LockedRect.RowPitch, 1, m_pFile) != 1)
        {
            return STATUS_FWRITE_FAILED;
        }

        pDeviceContext->Unmap(m_pStagingTexture, 0);

        return STATUS_OK;
    }

    FILE* m_pFile;
    ID3D11Texture2D* m_pBaseTexture;
    ID3D11Texture2D* m_pStagingTexture;
};

} // namespace D3D11
} // namespace SSAO
} // namespace GFSDK
