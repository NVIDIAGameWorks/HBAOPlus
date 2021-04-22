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
    m_TanHalfFovX = 1.f / (float)fabs(m(0,0));
    m_TanHalfFovY = 1.f / (float)fabs(m(1,1));

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
