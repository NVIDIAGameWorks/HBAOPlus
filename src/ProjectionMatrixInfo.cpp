/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#include "ProjectionMatrixInfo.h"
#include "MatrixView.h"

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::ProjectionMatrixInfo::Init(const GFSDK_SSAO_Matrix& ProjectionMatrix, APIConventions API)
{
    if (!IsValid(ProjectionMatrix))
    {
        return GFSDK_SSAO_INVALID_PROJECTION_MATRIX;
    }

    MatrixView m(ProjectionMatrix);

    // In matrices generated with D3DXMatrixPerspectiveFovRH
    // A = zf/(zn-zf)
    // B = zn*zf/(zn-zf)
    // C = -1
    float A = m(2,2);
    float B = m(3,2);
    float C = m(2,3);

    // In matrices generated with D3DXMatrixPerspectiveFovLH
    // A = -zf/(zn-zf)
    // B = zn*zf/(zn-zf)
    // C = 1
    if (C == 1.f)
    {
        A = -A;
    }

    // Rely on INFs to be generated in case of any divisions by zero
    m_ZNear = (API == API_GL) ? (B / (A - 1.f)) : (B / A);
    m_ZFar = B / (A + 1.f);

    // Some matrices may use negative m00 or m11 terms to flip X/Y axises
    m_TanHalfFovX = 1.f / fabs(m(0,0));
    m_TanHalfFovY = 1.f / fabs(m(1,1));

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
bool GFSDK::SSAO::ProjectionMatrixInfo::IsValid(const GFSDK_SSAO_Matrix& ProjectionMatrix)
{
    MatrixView m(ProjectionMatrix);

    // Do not check m(2,0) and m(2,1) to allow off-centered projections
    // Do not check m(2,2) to allow reverse infinite projections
    return (m(0,0) != 0.0f && m(0,1) == 0.0f && m(0,2) == 0.0f && m(0,3) == 0.0f &&
            m(1,0) == 0.0f && m(1,1) != 0.0f && m(1,2) == 0.0f && m(1,3) == 0.0f &&
            fabs(m(2,3)) == 1.0f &&
            m(3,0) == 0.0f && m(3,1) == 0.0f && m(3,2) != 0.0f && m(3,3) == 0.0f);
}
