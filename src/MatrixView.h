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

//--------------------------------------------------------------------------------
class MatrixView
{
public:
    MatrixView(const GFSDK_SSAO_Matrix& Matrix)
    {
        m_pFloat4x4 = Matrix.Data.Array;
        m_IsRowMajor = (Matrix.Layout == GFSDK_SSAO_ROW_MAJOR_ORDER);
    }

    float operator() (UINT Row, UINT Col) const
    {
        ASSERT(Row < 4);
        ASSERT(Col < 4);

        if (!m_IsRowMajor)
        {
            std::swap(Row, Col);
        }
        return m_pFloat4x4[Row * 4 + Col];
    }

private:
    const float* m_pFloat4x4;
    bool m_IsRowMajor;
};

} // namespace SSAO
} // namespace GFSDK
