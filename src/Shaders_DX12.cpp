/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#if SUPPORT_D3D12

#include "Shaders_DX12.h"

namespace GFSDK
{
namespace SSAO
{
namespace D3D12
{

#include "shaders/out/D3D11/FullScreenTriangle_VS.cpp"
#include "shaders/out/D3D11/CopyDepth_PS.cpp"
#include "shaders/out/D3D11/LinearizeDepth_PS.cpp"
#include "shaders/out/D3D11/DeinterleaveDepth_PS.cpp"
#include "shaders/out/D3D11/DebugNormals_PS.cpp"
#include "shaders/out/D3D11/ReconstructNormal_PS.cpp"
#include "shaders/out/D3D11/ReinterleaveAO_PS.cpp"
#include "shaders/out/D3D11/BlurX_PS.cpp"
#include "shaders/out/D3D11/BlurY_PS.cpp"
#include "shaders/out/D3D11/CoarseAO_PS.cpp"
#include "shaders/out/D3D11/CoarseAO_GS.cpp"

} // namespace D3D12
} // namespace SSAO
} // namespace GFSDK

#endif //SUPPORT_D3D12
