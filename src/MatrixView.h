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
