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
