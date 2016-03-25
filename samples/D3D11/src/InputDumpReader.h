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
#include "InputDumpWriter.h"

namespace GFSDK
{
namespace SSAO
{
namespace D3D11
{

class InputDumpReader
{
public:
    InputDumpReader()
        : m_pFile(NULL)
    {
    }

    ~InputDumpReader()
    {
        SafeCloseFile();
    }

    struct InputDumpData
    {
        InputDumpData()
        {
            memset(this, 0, sizeof(*this));
        }
        void Release()
        {
            SAFE_RELEASE(InputData.DepthData.pFullResDepthTextureSRV);
            SAFE_RELEASE(InputData.NormalData.pFullResNormalTextureSRV);
            SAFE_RELEASE(pDepthTexture);
            SAFE_RELEASE(pNormalTexture);
        }
        GFSDK_SSAO_InputData_D3D11 InputData;
        GFSDK_SSAO_Parameters Parameters;
        ID3D11Texture2D* pDepthTexture;
        ID3D11Texture2D* pNormalTexture;
    };

    enum Status
    {
        STATUS_OK,
        STATUS_NULL_ARGUMENT,
        STATUS_FOPEN_ERROR,
        STATUS_FREAD_ERROR,
        STATUS_FILE_VERSION_MISMATCH,
        STATUS_BUILD_VERSION_MISMATCH,
        STATUS_STRUCT_SIZE_MISMATCH,
        STATUS_CREATE_TEXTURE_ERROR,
        STATUS_CREATE_SRV_ERROR,
    };

    #define READ_STRUCT(S, FP)\
    {\
        UINT StructSize = 0;\
        if (fread(&StructSize, sizeof(StructSize), 1, FP) != 1)\
        {\
            return STATUS_FREAD_ERROR;\
        }\
        if (StructSize != sizeof(S))\
        {\
            return STATUS_STRUCT_SIZE_MISMATCH;\
        }\
        if (fread(&S, sizeof(S), 1, FP) != 1)\
        {\
            return STATUS_FREAD_ERROR;\
        }\
    }

    Status Read(
        ID3D11Device* pDevice,
        InputDumpData* pInputDumpData,
        const char* pDumpFilename)
    {
        InputDumpReader::Status Status = STATUS_OK;

        if (!pDevice || !pInputDumpData || !pDumpFilename)
        {
            return STATUS_NULL_ARGUMENT;
        }

        SafeCloseFile();
        if (fopen_s(&m_pFile, pDumpFilename, "rb") || !m_pFile)
        {
            return STATUS_FOPEN_ERROR;
        }

        UINT FileVersion = 0;
        if (fread(&FileVersion, sizeof(FileVersion), 1, m_pFile) != 1)
        {
            return STATUS_FREAD_ERROR;
        }

        if (FileVersion != InputDumpWriter::FILE_VERSION)
        {
            return STATUS_FILE_VERSION_MISMATCH;
        }

        GFSDK_SSAO_Version Version;
        READ_STRUCT(Version, m_pFile);

        const GFSDK_SSAO_Version BuildVersion;
        if (BuildVersion.Major != Version.Major ||
            BuildVersion.Minor != Version.Minor)
        {
            return STATUS_BUILD_VERSION_MISMATCH;
        }

        Status = ReadInputData(pDevice, pInputDumpData, m_pFile);
        if (Status != STATUS_OK)
        {
            return Status;
        }

        Status = ReadParameters(pDevice, pInputDumpData, m_pFile);
        if (Status != STATUS_OK)
        {
            return Status;
        }

        Status = ReadTexture(pDevice, pInputDumpData->pDepthTexture, pInputDumpData->InputData.DepthData.pFullResDepthTextureSRV, m_pFile);
        if (Status != STATUS_OK)
        {
            return Status;
        }

        if (pInputDumpData->InputData.NormalData.Enable)
        {
            Status = ReadTexture(pDevice, pInputDumpData->pNormalTexture, pInputDumpData->InputData.NormalData.pFullResNormalTextureSRV, m_pFile);
            if (Status != STATUS_OK)
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

    static Status ReadInputData(
        ID3D11Device* pDevice,
        InputDumpData* pInputDumpData,
        FILE *pFile)
    {
        GFSDK_SSAO_InputDepthData LoadedDepthData;
        READ_STRUCT(LoadedDepthData, pFile);

        GFSDK_SSAO_InputNormalData LoadedNormalData;
        READ_STRUCT(LoadedNormalData, pFile);

        GFSDK_SSAO_InputData_D3D11& InputData = pInputDumpData->InputData;
        memset(&InputData, 0, sizeof(InputData));
        memcpy(&InputData.DepthData, &LoadedDepthData, sizeof(LoadedDepthData));
        memcpy(&InputData.NormalData, &LoadedNormalData, sizeof(LoadedNormalData));

        return STATUS_OK;
    }

    static Status ReadParameters(
        ID3D11Device* pDevice,
        InputDumpData* pInputDumpData,
        FILE *pFile)
    {
        GFSDK_SSAO_Parameters LoadedParameters;
        READ_STRUCT(LoadedParameters, pFile);

        GFSDK_SSAO_Parameters& Parameters = pInputDumpData->Parameters;
        memset(&Parameters, 0, sizeof(Parameters));
        memcpy(&Parameters, &LoadedParameters, sizeof(LoadedParameters));

        return STATUS_OK;
    }

protected:
    static Status ReadTexture(
        ID3D11Device* pDevice,
        ID3D11Texture2D*& pOutTexture,
        ID3D11ShaderResourceView*& pOutSRV,
        FILE *pFile)
    {
        D3D11_TEXTURE2D_DESC TextureDesc;
        if (fread(&TextureDesc, sizeof(TextureDesc), 1, pFile) != 1)
        {
            return STATUS_FREAD_ERROR;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc;
        if (fread(&SrvDesc, sizeof(SrvDesc), 1, pFile) != 1)
        {
            return STATUS_FREAD_ERROR;
        }

        UINT RowPitch = 0;
        if (fread(&RowPitch, sizeof(RowPitch), 1, pFile) != 1)
        {
            return STATUS_FREAD_ERROR;
        }

        BYTE* pData = new BYTE[TextureDesc.Height * RowPitch];
        assert(pData);

        if (fread(pData, TextureDesc.Height * RowPitch, 1, pFile) != 1)
        {
            return STATUS_FREAD_ERROR;
        }

        TextureDesc.Usage            = D3D11_USAGE_IMMUTABLE;
        TextureDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
        TextureDesc.CPUAccessFlags   = 0;
        TextureDesc.MiscFlags        = 0;

        D3D11_SUBRESOURCE_DATA SubresourceData;
        SubresourceData.pSysMem          = pData;
        SubresourceData.SysMemPitch      = RowPitch;
        SubresourceData.SysMemSlicePitch = 0;

        assert(!pOutTexture);
        if (pDevice->CreateTexture2D(&TextureDesc, &SubresourceData, &pOutTexture) != S_OK)
        {
            return STATUS_CREATE_TEXTURE_ERROR;
        }

        delete[] pData;

        assert(!pOutSRV);
        if (pDevice->CreateShaderResourceView(pOutTexture, &SrvDesc, &pOutSRV) != S_OK)
        {
            return STATUS_CREATE_SRV_ERROR;
        }

        return STATUS_OK;
    }

    FILE* m_pFile;
};

class InputDumpReader_2 : public InputDumpReader
{
public:
    static Status Read(
        ID3D11Device* pDevice,
        InputDumpReader::InputDumpData* pInputDumpData,
        const char* pDumpFilename)
    {
        Status ReadStatus = STATUS_OK;

        if (!pDevice || !pInputDumpData || !pDumpFilename)
        {
            return STATUS_NULL_ARGUMENT;
        }

        FILE *pFile = NULL;
        if (fopen_s(&pFile, pDumpFilename, "rb") || !pFile)
        {
            return STATUS_FOPEN_ERROR;
        }

        UINT FileVersion = 0;
        if (fread(&FileVersion, sizeof(FileVersion), 1, pFile) != 1)
        {
            return STATUS_FREAD_ERROR;
        }

        if (FileVersion != 2)
        {
            return STATUS_FILE_VERSION_MISMATCH;
        }

        GFSDK_SSAO_Version Version;
        if (fread(&Version, sizeof(Version), 1, pFile) != 1)
        {
            return STATUS_FREAD_ERROR;
        }

        ReadStatus = ReadStructs(pFile, pInputDumpData);
        if (ReadStatus != STATUS_OK)
        {
            return ReadStatus;
        }

        ReadStatus = InputDumpReader::ReadTexture(pDevice, pInputDumpData->pDepthTexture, pInputDumpData->InputData.DepthData.pFullResDepthTextureSRV, pFile);
        if (ReadStatus != STATUS_OK)
        {
            return ReadStatus;
        }

        if (pInputDumpData->InputData.NormalData.Enable)
        {
            ReadStatus = InputDumpReader::ReadTexture(pDevice, pInputDumpData->pNormalTexture, pInputDumpData->InputData.NormalData.pFullResNormalTextureSRV, pFile);
            if (ReadStatus != STATUS_OK)
            {
                return ReadStatus;
            }
        }

        fclose(pFile);

        return STATUS_OK;
    }

private:
    struct InputViewport
    {
        GFSDK_SSAO_UINT TopLeftX;
        GFSDK_SSAO_UINT TopLeftY;
        GFSDK_SSAO_UINT Width;
        GFSDK_SSAO_UINT Height;
        GFSDK_SSAO_FLOAT MinDepth;
        GFSDK_SSAO_FLOAT MaxDepth;

        InputViewport()
            : TopLeftX(0)
            , TopLeftY(0)
            , Width(0)
            , Height(0)
            , MinDepth(0.f)
            , MaxDepth(1.f)
        {
        }
    };

    struct Matrix
    {
        Matrix()
        {
            memset(this, 0, sizeof(*this));
        }
        Matrix(const GFSDK_SSAO_FLOAT* pMatrix)
        {
            memcpy(&Data, pMatrix, sizeof(Data));
        }
        GFSDK_SSAO_FLOAT Data[16];
    };

    struct InputDepthData
    {
        GFSDK_SSAO_DepthTextureType     DepthTextureType;           // HARDWARE_DEPTHS, HARDWARE_DEPTHS_SUB_RANGE or VIEW_DEPTHS
        Matrix                          ProjectionMatrix;           // 4x4 perspective matrix from the depth generation pass
        GFSDK_SSAO_MatrixLayout         ProjectionMatrixLayout;     // Memory layout of the projection matrix
        GFSDK_SSAO_FLOAT                MetersToViewSpaceUnits;     // DistanceInViewSpaceUnits = MetersToViewSpaceUnits * DistanceInMeters

        InputDepthData()
            : DepthTextureType(GFSDK_SSAO_HARDWARE_DEPTHS)
            , ProjectionMatrixLayout(GFSDK_SSAO_ROW_MAJOR_ORDER)
            , MetersToViewSpaceUnits(1.f)
         {
         }
    };

    struct InputDepthData_D3D11 : InputDepthData
    {
        ID3D11ShaderResourceView*       pFullResDepthTextureSRV;    // Full-resolution depth texture
        InputViewport                   Viewport;                   // Viewport from the depth generation pass

        InputDepthData_D3D11()
            : pFullResDepthTextureSRV(NULL)
         {
         }
    };

    struct InputNormalData
    {
        GFSDK_SSAO_BOOL                 Enable;                         // To use the provided normals (instead of reconstructed ones)
        Matrix                          WorldToViewMatrix;              // 4x4 WorldToView matrix from the depth generation pass
        GFSDK_SSAO_MatrixLayout         WorldToViewMatrixLayout;        // Memory layout of the WorldToView matrix
        GFSDK_SSAO_FLOAT                DecodeScale;                    // Optional pre-matrix scale
        GFSDK_SSAO_FLOAT                DecodeBias;                     // Optional pre-matrix bias

        InputNormalData()
            : Enable(false)
            , WorldToViewMatrixLayout(GFSDK_SSAO_ROW_MAJOR_ORDER)
            , DecodeScale(1.f)
            , DecodeBias(0.f)
        {
        }
    };

    struct InputNormalData_D3D11 : InputNormalData
    {
        ID3D11ShaderResourceView*   pFullResNormalTextureSRV;       // Full-resolution world-space normal texture

        InputNormalData_D3D11()
            : pFullResNormalTextureSRV(NULL)
        {
        }
    };

    struct InputData_D3D11
    {
        InputDepthData_D3D11         DepthData;          // Required
        InputNormalData_D3D11        NormalData;         // Optional GBuffer normals
    };


    struct OutputParameters
    {
        GFSDK_SSAO_BlendMode            BlendMode;                  // Blend mode used to composite the AO to the output render target

        OutputParameters()
            : BlendMode(GFSDK_SSAO_OVERWRITE_RGB)
        {
        }
    };

    struct CustomBlendState_D3D11
    {
        ID3D11BlendState*               pBlendState;                // Custom blend state to composite the AO with
        const GFSDK_SSAO_FLOAT*         pBlendFactor;               // Relevant only if pBlendState uses D3D11_BLEND_BLEND_FACTOR

        CustomBlendState_D3D11()
            : pBlendState(NULL)
            , pBlendFactor(NULL)
        {
        }
    };

    struct Parameters_D3D11
    {
        GFSDK_SSAO_FLOAT                Radius;                     // The AO radius in meters
        GFSDK_SSAO_FLOAT                Bias;                       // To hide low-tessellation artifacts // 0.0~1.0
        GFSDK_SSAO_FLOAT                DetailAO;                   // Scale factor for the detail AO, the greater the darker // 0.0~2.0
        GFSDK_SSAO_FLOAT                CoarseAO;                   // Scale factor for the coarse AO, the greater the darker // 0.0~2.0
        GFSDK_SSAO_FLOAT                PowerExponent;              // The final AO output is pow(AO, powerExponent)
        GFSDK_SSAO_DepthStorage         DepthStorage;               // Quality / performance tradeoff
        GFSDK_SSAO_DepthClampMode       DepthClampMode;             // To hide possible false-occlusion artifacts near screen borders
        GFSDK_SSAO_DepthThreshold       DepthThreshold;             // Optional Z threshold, to hide possible depth-precision artifacts
        GFSDK_SSAO_BlurParameters       Blur;                       // Optional AO blur, to blur the AO before compositing it

        Parameters_D3D11()
            : Radius(1.f)
            , Bias(0.1f)
            , DetailAO(0.f)
            , CoarseAO(1.f)
            , PowerExponent(2.f)
            , DepthStorage(GFSDK_SSAO_FP32_VIEW_DEPTHS)
            , DepthClampMode(GFSDK_SSAO_CLAMP_TO_EDGE)
        {
        }
    };

    static Status ReadStructs(FILE *pFile, InputDumpReader::InputDumpData* pOut)
    {
        InputDepthData InputDepthData;
        if (fread(&InputDepthData, sizeof(InputDepthData), 1, pFile) != 1)
        {
            return STATUS_FREAD_ERROR;
        }

        InputViewport InputViewport;
        if (fread(&InputViewport, sizeof(InputViewport), 1, pFile) != 1)
        {
            return STATUS_FREAD_ERROR;
        }

        InputNormalData InputNormalData;
        if (fread(&InputNormalData, sizeof(InputNormalData), 1, pFile) != 1)
        {
            return STATUS_FREAD_ERROR;
        }

        GFSDK_SSAO_InputData_D3D11& OutInputData = pOut->InputData;
        memset(&OutInputData, 0, sizeof(OutInputData));

        OutInputData.DepthData.DepthTextureType = InputDepthData.DepthTextureType;
        memcpy(&OutInputData.DepthData.ProjectionMatrix.Data, &InputDepthData.ProjectionMatrix, sizeof(InputDepthData.ProjectionMatrix));
        OutInputData.DepthData.ProjectionMatrix.Layout = InputDepthData.ProjectionMatrixLayout;
        OutInputData.DepthData.MetersToViewSpaceUnits = InputDepthData.MetersToViewSpaceUnits;
        OutInputData.DepthData.Viewport.Enable = 0;

        OutInputData.NormalData.Enable = InputNormalData.Enable;
        memcpy(&OutInputData.NormalData.WorldToViewMatrix.Data, &InputNormalData.WorldToViewMatrix, sizeof(InputNormalData.WorldToViewMatrix));
        OutInputData.NormalData.WorldToViewMatrix.Layout = InputNormalData.WorldToViewMatrixLayout;
        OutInputData.NormalData.DecodeScale = InputNormalData.DecodeScale;
        OutInputData.NormalData.DecodeBias = InputNormalData.DecodeBias;

        Parameters_D3D11 Parameters;
        if (fread(&Parameters, sizeof(Parameters), 1, pFile) != 1)
        {
            return STATUS_FREAD_ERROR;
        }

        pOut->Parameters.Radius = Parameters.Radius;
        pOut->Parameters.Bias = Parameters.Bias;
        pOut->Parameters.DetailAO = Parameters.DetailAO;
        pOut->Parameters.CoarseAO = Parameters.CoarseAO;
        pOut->Parameters.DepthStorage = Parameters.DepthStorage;
        pOut->Parameters.PowerExponent = Parameters.PowerExponent;
        pOut->Parameters.DepthClampMode = Parameters.DepthClampMode;
        pOut->Parameters.DepthThreshold = Parameters.DepthThreshold;
        pOut->Parameters.Blur = Parameters.Blur;

        return STATUS_OK;
    }
};

} // namespace D3D11
} // namespace SSAO
} // namespace GFSDK
