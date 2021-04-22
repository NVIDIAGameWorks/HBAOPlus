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

#pragma once
#include "Common.h"

namespace GFSDK
{
namespace SSAO
{

enum APIConventions
{
    API_D3D11,
    API_GL
};

class ProjectionMatrixInfo
{
public:
    ProjectionMatrixInfo()
        : m_ZNear(0.f)
        , m_ZFar(0.f)
        , m_TanHalfFovX(0.f)
        , m_TanHalfFovY(0.f)
    {
    }

    GFSDK_SSAO_Status Init(const GFSDK_SSAO_Matrix& ProjectionMatrix, APIConventions API);
    static bool IsValid(const GFSDK_SSAO_Matrix& ProjectionMatrix);

    // Clamp to EPSILON to avoid any divisions by 0.f
    float GetInverseZNear() const
    {
        return Max(1.f / m_ZNear, EPSILON);
    }
    float GetInverseZFar() const
    {
        return Max(1.f / m_ZFar, EPSILON);
    }

    float GetTanHalfFovX() const
    {
        return m_TanHalfFovX;
    }
    float GetTanHalfFovY() const
    {
        return m_TanHalfFovY;
    }

    void GetDepthRange(GFSDK_SSAO_ProjectionMatrixDepthRange* pDepthRange)
    {
        pDepthRange->ZNear = 1.f / GetInverseZNear();
        pDepthRange->ZFar = 1.f / GetInverseZFar();
    }

private:
    float m_ZNear;
    float m_ZFar;
    float m_TanHalfFovX;
    float m_TanHalfFovY;
};

} // namespace SSAO
} // namespace GFSDK
