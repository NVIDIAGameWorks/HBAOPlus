/* 
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

/*====================================================================================================
-------------------------------------------------------------------------------------------
                                           HBAO+                                           
-------------------------------------------------------------------------------------------

HBAO+ is a SSAO algorithm designed to achieve high efficiency on DX11 GPUs.
The algorithm is based on HBAO [Bavoil and Sainz 2008], with the following differences:

(1.) To minimize cache trashing, HBAO+ does not use any randomization texture.
Instead, the algorithm uses an Interleaved Rendering approach, generating the AO
in multiple passes with a unique jitter value per pass [Bavoil and Jansen 2013].

(2.) To avoid over-occlusion artifacts, HBAO+ uses a simpler AO approximation than HBAO,
similar to "Scalable Ambient Obscurance" [McGuire et al. 2012] [Bukowski et al. 2012].

(3.) To minimize flickering, the HBAO+ is always rendered in full resolution,
from full-resolution depths.

(4.) To reduce halo artifacts behind foreground objects, HBAO+ can take as input a second depth layer, 
using the multi-layer SSAO approach from [Bavoil and Sainz 2009] and [McGuire et al. 2013].

[Bavoil et al. 2008] "Image-Space Horizon-Based Ambient Occlusion"
http://www.nvidia.com/object/siggraph-2008-HBAO.html

[Bavoil and Sainz 2009] "Multi-Layer Dual-Resolution Screen-Space Ambient Occlusion"
https://dl.acm.org/citation.cfm?id=1598035

[McGuire et al. 2012] "Scalable Ambient Obscurance"
http://graphics.cs.williams.edu/papers/SAOHPG12/

[Bukowski et al. 2012] "Scalable High-Quality Motion Blur and Ambient Occlusion"
http://graphics.cs.williams.edu/papers/VVSIGGRAPH12/

[Bavoil and Jansen 2013] "Particle Shadows & Cache-Efficient Post-Processing"
https://developer.nvidia.com/gdc-2013

[McGuire et al. 2013] "Lighting Deep G-Buffers: Single-Pass, Layered Depth Images with Minimum Separation Applied to Indirect Illumination"
http://research.nvidia.com/publication/lighting-deep-g-buffers-single-pass-layered-depth-images-minimum-separation-applied

-------------------------------------------------------------------------------------------
                                           MSAA                                            
-------------------------------------------------------------------------------------------

(1.) The input depth & normal textures are required to have matching dimensions and MSAA sample count.

(2.) The output render target can have arbitrary dimensions and MSAA sample count.

(3.) If the input textures are MSAA, only sample 0 is used to render the AO.

(4.) If the output render target is MSAA, a per-pixel AO value is written to all samples.

-------------------------------------------------------------------------------------------
                                       PERFORMANCE                                        
-------------------------------------------------------------------------------------------

In 3840x2160 (4K-UHD) on GeForce GTX 1080, using 1xAA HARDWARE_DEPTHS as input, STEP_COUNT_4,
BLUR_RADIUS_4, and FP16_VIEW_DEPTHS, the RenderAO call takes:
- 2.4 ms / frame & up to 150 MB of video memory with 1 input depth texture
- 3.6 ms / frame & up to 197 MB of video memory with 2 input depth textures.

-------------------------------------------------------------------------------------------
                                    INTEGRATION EXAMPLE                                    
-------------------------------------------------------------------------------------------

(1.) INITIALIZE THE LIBRARY

    GFSDK_SSAO_CustomHeap CustomHeap;
    CustomHeap.new_ = ::operator new;
    CustomHeap.delete_ = ::operator delete;

    GFSDK_SSAO_Status status;
    GFSDK_SSAO_Context_D3D11* pAOContext;
    status = GFSDK_SSAO_CreateContext_D3D11(pD3D11Device, &pAOContext, &CustomHeap);
    assert(status == GFSDK_SSAO_OK); // HBAO+ requires feature level 11_0 or above

(2.) SET INPUT DEPTHS

    GFSDK_SSAO_InputData_D3D11 Input;
    Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
    Input.DepthData.pFullResDepthTextureSRV = pDepthStencilTextureSRV;
    Input.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4(pProjectionMatrix);
    Input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
    Input.DepthData.MetersToViewSpaceUnits = SceneScale;

(3.) SET AO PARAMETERS

    GFSDK_SSAO_Parameters Params;
    Params.Radius = 1.f;
    Params.Bias = 0.1f;
    Params.PowerExponent = 2.f;
    Params.Blur.Enable = true;
    Params.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
    Params.Blur.Sharpness = 16.f;

(4.) SET RENDER TARGET

    GFSDK_SSAO_Output_D3D11 Output;
    Output.pRenderTargetView = pOutputColorRTV;
    Output.Blend.Mode = GFSDK_SSAO_OVERWRITE_RGB;

(5.) RENDER AO

    status = pAOContext->RenderAO(pD3D11Context, Input, Params, Output);
    assert(status == GFSDK_SSAO_OK);

-------------------------------------------------------------------------------------------
                                 DOCUMENTATION AND SUPPORT                                 
-------------------------------------------------------------------------------------------

    See the documentation for more information on the input requirements and the parameters.
    For any feedback or questions about this library, please contact devsupport@nvidia.com.

====================================================================================================*/

#pragma once
#pragma pack(push,8) // Make sure we have consistent structure packings

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*====================================================================================================
   Entry-point declarations.
====================================================================================================*/

#if defined(ANDROID) || defined(LINUX) || defined(MACOSX)
#define GFSDK_SSAO_CDECL
#define GFSDK_SSAO_EXPORT
#define GFSDK_SSAO_STDCALL
#else
#define GFSDK_SSAO_CDECL __cdecl
#define GFSDK_SSAO_EXPORT __declspec(dllexport)
#define GFSDK_SSAO_STDCALL __stdcall
#endif

#if GFSDK_SSAO_DYNAMIC_LOAD_LIBRARY

#define GFSDK_SSAO_DECL(RETURN_TYPE, FUNCTION_NAME, ...) typedef RETURN_TYPE (*PFN_##FUNCTION_NAME)(__VA_ARGS__); extern PFN_##FUNCTION_NAME FUNCTION_NAME
#define GFSDK_SSAO_VERSION_ARGUMENT GFSDK_SSAO_Version HeaderVersion
#define GFSDK_SSAO_CUSTOM_HEAP_ARGUMENT const GFSDK_SSAO_CustomHeap* pCustomHeap

#else

#if !defined(_WINDLL) || !_WINDLL
#define GFSDK_SSAO_DECL(RETURN_TYPE, FUNCTION_NAME, ...) extern "C" RETURN_TYPE GFSDK_SSAO_CDECL FUNCTION_NAME(__VA_ARGS__)
#else
#define GFSDK_SSAO_DECL(RETURN_TYPE, FUNCTION_NAME, ...) extern "C" GFSDK_SSAO_EXPORT RETURN_TYPE GFSDK_SSAO_CDECL FUNCTION_NAME(__VA_ARGS__)
#endif

#define GFSDK_SSAO_VERSION_ARGUMENT GFSDK_SSAO_Version HeaderVersion = GFSDK_SSAO_Version()
#define GFSDK_SSAO_CUSTOM_HEAP_ARGUMENT const GFSDK_SSAO_CustomHeap* pCustomHeap = NULL

#endif

/*====================================================================================================
   Forward declarations.
====================================================================================================*/

typedef unsigned int GFSDK_SSAO_BOOL;
typedef unsigned int GFSDK_SSAO_UINT;
typedef float GFSDK_SSAO_FLOAT;
typedef size_t GFSDK_SSAO_SIZE_T;
typedef uint64_t GFSDK_SSAO_UINT64;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;
struct ID3D11BlendState;
struct ID3D11DepthStencilState;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;

struct ID3D12Device;
struct ID3D12CommandQueue;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct D3D12_BLEND_DESC;

/*====================================================================================================
   Build version.
====================================================================================================*/

struct GFSDK_SSAO_Version
{
    GFSDK_SSAO_Version()
        : Major(4)
        , Minor(0)
        , Branch(0)
        , Revision(23827312)
    {
    }

    GFSDK_SSAO_UINT Major;
    GFSDK_SSAO_UINT Minor;
    GFSDK_SSAO_UINT Branch;
    GFSDK_SSAO_UINT Revision;
};

/*====================================================================================================
   Enums.
====================================================================================================*/

enum GFSDK_SSAO_Status
{
    GFSDK_SSAO_OK,                                          // Success
    GFSDK_SSAO_VERSION_MISMATCH,                            // The header version number does not match the DLL version number
    GFSDK_SSAO_NULL_ARGUMENT,                               // One of the required argument pointers is NULL
    GFSDK_SSAO_INVALID_PROJECTION_MATRIX,                   // The projection matrix is not valid
    GFSDK_SSAO_INVALID_WORLD_TO_VIEW_MATRIX,                // The world-to-view matrix is not valid (transposing it may help)
    GFSDK_SSAO_INVALID_NORMAL_TEXTURE_RESOLUTION,           // The normal-texture resolution does not match the depth-texture resolution
    GFSDK_SSAO_INVALID_NORMAL_TEXTURE_SAMPLE_COUNT,         // The normal-texture sample count does not match the depth-texture sample count
    GFSDK_SSAO_INVALID_VIEWPORT_DIMENSIONS,                 // One of the viewport dimensions (width or height) is 0
    GFSDK_SSAO_INVALID_VIEWPORT_DEPTH_RANGE,                // The viewport depth range is not a sub-range of [0.f,1.f]
    GFSDK_SSAO_INVALID_SECOND_DEPTH_TEXTURE_RESOLUTION,     // The resolution of the second depth texture does not match the one of the first depth texture
    GFSDK_SSAO_INVALID_SECOND_DEPTH_TEXTURE_SAMPLE_COUNT,   // The sample count of the second depth texture does not match the one of the first depth texture
    GFSDK_SSAO_MEMORY_ALLOCATION_FAILED,                    // Failed to allocate memory on the heap
    GFSDK_SSAO_INVALID_DEPTH_STENCIL_RESOLUTION,            // The depth-stencil resolution does not match the output render-target resolution
    GFSDK_SSAO_INVALID_DEPTH_STENCIL_SAMPLE_COUNT,          // The depth-stencil sample count does not match the output render-target sample count
    GFSDK_SSAO_D3D_FEATURE_LEVEL_NOT_SUPPORTED,             // The current D3D11 feature level is lower than 11_0
    GFSDK_SSAO_D3D_RESOURCE_CREATION_FAILED,                // A resource-creation call has failed (running out of memory?)
    GFSDK_SSAO_D3D12_UNSUPPORTED_DEPTH_CLAMP_MODE,          // CLAMP_TO_BORDER is used (implemented on D3D11 & GL, but not on D3D12)
    GFSDK_SSAO_D3D12_INVALID_HEAP_TYPE,                     // One of the heaps provided to GFSDK_SSAO_CreateContext_D3D12 has an unexpected type
    GFSDK_SSAO_D3D12_INSUFFICIENT_DESCRIPTORS,              // One of the heaps provided to GFSDK_SSAO_CreateContext_D3D12 has an insufficient number of descriptors
    GFSDK_SSAO_D3D12_INVALID_NODE_MASK,                     // NodeMask has more than one bit set. HBAO+ only supports operation on one D3D12 device node.
    GFSDK_SSAO_NO_SECOND_LAYER_PROVIDED                     // FullResDepthTexture2ndLayerSRV is not set, but DualLayerAO is enabled.
};

enum GFSDK_SSAO_DepthTextureType
{
    GFSDK_SSAO_HARDWARE_DEPTHS,                             // Non-linear depths in the range [0.f,1.f]
    GFSDK_SSAO_HARDWARE_DEPTHS_SUB_RANGE,                   // Non-linear depths in the range [Viewport.MinDepth,Viewport.MaxDepth]
    GFSDK_SSAO_VIEW_DEPTHS,                                 // Linear depths in the range [ZNear,ZFar]
};

enum GFSDK_SSAO_BlendMode
{
    GFSDK_SSAO_OVERWRITE_RGB,                               // Overwrite the destination RGB with the AO, preserving alpha
    GFSDK_SSAO_MULTIPLY_RGB,                                // Multiply the AO over the destination RGB, preserving alpha
    GFSDK_SSAO_CUSTOM_BLEND,                                // Composite the AO using a custom blend state
};

enum GFSDK_SSAO_DepthStencilMode
{
    GFSDK_SSAO_DISABLED_DEPTH_STENCIL,                      // Composite the AO without any depth-stencil testing
    GFSDK_SSAO_CUSTOM_DEPTH_STENCIL,                        // Composite the AO with a custom depth-stencil state
};

enum GFSDK_SSAO_BlurRadius
{
    GFSDK_SSAO_BLUR_RADIUS_2,                               // Kernel radius = 2 pixels
    GFSDK_SSAO_BLUR_RADIUS_4,                               // Kernel radius = 4 pixels (recommended)
};

enum GFSDK_SSAO_DepthStorage
{
    GFSDK_SSAO_FP16_VIEW_DEPTHS,                            // Store the internal view depths in FP16 (recommended)
    GFSDK_SSAO_FP32_VIEW_DEPTHS,                            // Store the internal view depths in FP32 (slower)
};

enum GFSDK_SSAO_StepCount
{
    GFSDK_SSAO_STEP_COUNT_4,                                // Use 4 steps per sampled direction (same as in HBAO+ 3.x)
    GFSDK_SSAO_STEP_COUNT_8,                                // Use 8 steps per sampled direction (slower, to reduce banding artifacts)
};

enum GFSDK_SSAO_DepthClampMode
{
    GFSDK_SSAO_CLAMP_TO_EDGE,                               // Use clamp-to-edge when sampling depth (may cause false occlusion near screen borders)
    GFSDK_SSAO_CLAMP_TO_BORDER,                             // Use clamp-to-border when sampling depth (may cause halos near screen borders)
};

enum GFSDK_SSAO_MatrixLayout
{
    GFSDK_SSAO_ROW_MAJOR_ORDER,                             // The matrix is stored as Row[0],Row[1],Row[2],Row[3]
    GFSDK_SSAO_COLUMN_MAJOR_ORDER,                          // The matrix is stored as Col[0],Col[1],Col[2],Col[3]
};

enum GFSDK_SSAO_RenderMask
{
    GFSDK_SSAO_DRAW_Z                              = (1 << 0),  // Linearize the input depths
    GFSDK_SSAO_DRAW_AO                             = (1 << 1),  // Render AO based on pre-linearized depths
    GFSDK_SSAO_DRAW_DEBUG_N                        = (1 << 2),  // Render the internal view normals (for debugging)
    GFSDK_SSAO_DRAW_DEBUG_X                        = (1 << 3),  // Render the X component as grayscale
    GFSDK_SSAO_DRAW_DEBUG_Y                        = (1 << 4),  // Render the Y component as grayscale
    GFSDK_SSAO_DRAW_DEBUG_Z                        = (1 << 5),  // Render the Z component as grayscale
    GFSDK_SSAO_RENDER_AO                           = GFSDK_SSAO_DRAW_Z | GFSDK_SSAO_DRAW_AO,
    GFSDK_SSAO_RENDER_DEBUG_NORMAL                 = GFSDK_SSAO_DRAW_Z | GFSDK_SSAO_DRAW_DEBUG_N,
    GFSDK_SSAO_RENDER_DEBUG_NORMAL_X               = GFSDK_SSAO_DRAW_Z | GFSDK_SSAO_DRAW_DEBUG_N | GFSDK_SSAO_DRAW_DEBUG_X,
    GFSDK_SSAO_RENDER_DEBUG_NORMAL_Y               = GFSDK_SSAO_DRAW_Z | GFSDK_SSAO_DRAW_DEBUG_N | GFSDK_SSAO_DRAW_DEBUG_Y,
    GFSDK_SSAO_RENDER_DEBUG_NORMAL_Z               = GFSDK_SSAO_DRAW_Z | GFSDK_SSAO_DRAW_DEBUG_N | GFSDK_SSAO_DRAW_DEBUG_Z,
};

/*====================================================================================================
   Input/output textures.
====================================================================================================*/

struct GFSDK_SSAO_ShaderResourceView_D3D12
{
    GFSDK_SSAO_ShaderResourceView_D3D12()
        : pResource(NULL)
        , GpuHandle(0)
    {
    }
    ID3D12Resource*     pResource;
    GFSDK_SSAO_UINT64   GpuHandle;
};

struct GFSDK_SSAO_RenderTargetView_D3D12
{
    GFSDK_SSAO_RenderTargetView_D3D12()
        : pResource(NULL)
        , CpuHandle(0)
    {
    }
    ID3D12Resource*     pResource;
    GFSDK_SSAO_SIZE_T   CpuHandle;
};

/*====================================================================================================
   Input data.
====================================================================================================*/

//---------------------------------------------------------------------------------------------------
// 4x4 Matrix.
//---------------------------------------------------------------------------------------------------
struct GFSDK_SSAO_Float4x4
{
    GFSDK_SSAO_Float4x4()
    {
        memset(&Array, 0, sizeof(Array));
    }
    GFSDK_SSAO_Float4x4(const GFSDK_SSAO_FLOAT* pMatrix)
    {
        memcpy(&Array, pMatrix, sizeof(Array));
    }
    GFSDK_SSAO_FLOAT Array[16];
};

struct GFSDK_SSAO_Matrix
{
    GFSDK_SSAO_Float4x4         Data;                       // 4x4 matrix stored as an array of 16 floats
    GFSDK_SSAO_MatrixLayout     Layout;                     // Memory layout of the matrix

    GFSDK_SSAO_Matrix()
        : Layout(GFSDK_SSAO_ROW_MAJOR_ORDER)
    {
    }
};

//---------------------------------------------------------------------------------------------------
// [Optional] Input viewport.
//
// Remarks:
//    * The Viewport defines a sub-area of the input & output full-resolution textures to be sourced and rendered to.
//      Only the depth pixels within the viewport sub-area contribute to the RenderAO output.
//    * The Viewport's MinDepth and MaxDepth values are ignored except if DepthTextureType is HARDWARE_DEPTHS_SUB_RANGE.
//    * If Enable is false, a default input viewport is used with:
//          TopLeftX = 0
//          TopLeftY = 0
//          Width    = InputDepthTexture.Width
//          Height   = InputDepthTexture.Height
//          MinDepth = 0.f
//          MaxDepth = 1.f
//---------------------------------------------------------------------------------------------------
struct GFSDK_SSAO_InputViewport
{
    GFSDK_SSAO_BOOL     Enable;                             // To use the provided viewport data (instead of the default viewport)
    GFSDK_SSAO_UINT     TopLeftX;                           // X coordinate of the top-left corner of the viewport rectangle, in pixels
    GFSDK_SSAO_UINT     TopLeftY;                           // Y coordinate of the top-left corner of the viewport rectangle, in pixels
    GFSDK_SSAO_UINT     Width;                              // The width of the viewport rectangle, in pixels
    GFSDK_SSAO_UINT     Height;                             // The height of the viewport rectangle, in pixels
    GFSDK_SSAO_FLOAT    MinDepth;                           // The minimum depth for GFSDK_SSAO_HARDWARE_DEPTHS_SUB_RANGE
    GFSDK_SSAO_FLOAT    MaxDepth;                           // The maximum depth for GFSDK_SSAO_HARDWARE_DEPTHS_SUB_RANGE

    GFSDK_SSAO_InputViewport()
        : Enable(false)
        , TopLeftX(0)
        , TopLeftY(0)
        , Width(0)
        , Height(0)
        , MinDepth(0.f)
        , MaxDepth(1.f)
    {
    }
};

//---------------------------------------------------------------------------------------------------
// Input depth data.
//
// Requirements:
//    * View-space depths (linear) are required to be non-multisample.
//    * Hardware depths (non-linear) can be multisample or non-multisample.
//    * The projection matrix must have the following form, with |P23| == 1.f:
//       { P00, 0.f, 0.f, 0.f }
//       { 0.f, P11, 0.f, 0.f }
//       { P20, P21, P22, P23 }
//       { 0.f, 0.f, P32, 0.f }
//
// Remark:
//    * MetersToViewSpaceUnits is used to convert the AO radius parameter from meters to view-space units,
//      as well as to convert the blur sharpness parameter from inverse meters to inverse view-space units.
//---------------------------------------------------------------------------------------------------

struct GFSDK_SSAO_InputDepthData
{
    GFSDK_SSAO_DepthTextureType     DepthTextureType;           // HARDWARE_DEPTHS, HARDWARE_DEPTHS_SUB_RANGE or VIEW_DEPTHS
    GFSDK_SSAO_Matrix               ProjectionMatrix;           // 4x4 perspective matrix from the depth generation pass
    GFSDK_SSAO_FLOAT                MetersToViewSpaceUnits;     // DistanceInViewSpaceUnits = MetersToViewSpaceUnits * DistanceInMeters
    GFSDK_SSAO_InputViewport        Viewport;                   // Viewport from the depth generation pass

    GFSDK_SSAO_InputDepthData()
        : DepthTextureType(GFSDK_SSAO_HARDWARE_DEPTHS)
        , MetersToViewSpaceUnits(1.f)
    {
    }
};

struct GFSDK_SSAO_InputDepthData_D3D12 : GFSDK_SSAO_InputDepthData
{
    GFSDK_SSAO_ShaderResourceView_D3D12 FullResDepthTextureSRV;         // Full-resolution depth texture
    GFSDK_SSAO_ShaderResourceView_D3D12 FullResDepthTexture2ndLayerSRV; // Full-resolution depth texture for the second layer
};

struct GFSDK_SSAO_InputDepthData_D3D11 : GFSDK_SSAO_InputDepthData
{
    ID3D11ShaderResourceView*       pFullResDepthTextureSRV;            // Full-resolution depth texture
    ID3D11ShaderResourceView*       pFullResDepthTexture2ndLayerSRV;    // Full-resolution depth texture for the second layer

    GFSDK_SSAO_InputDepthData_D3D11()
    {
        pFullResDepthTextureSRV = NULL;
        pFullResDepthTexture2ndLayerSRV = NULL;
    }
};

//---------------------------------------------------------------------------------------------------
// [Optional] Input normal data.
//
// Requirements:
//    * The normal texture is required to contain world-space normals in RGB.
//    * The normal texture must have the same resolution and MSAA sample count as the input depth texture.
//    * The view-space Y & Z axis are assumed to be pointing up & forward respectively (left-handed projection).
//    * The WorldToView matrix is assumed to not contain any non-uniform scaling.
//    * The WorldView matrix must have the following form:
//       { M00, M01, M02, 0.f }
//       { M10, M11, M12, 0.f }
//       { M20, M21, M22, 0.f }
//       { M30, M31, M32, M33 }
//
// Remarks:
//    * The actual view-space normal used for the AO rendering is:
//      N = normalize( mul( FetchedNormal.xyz * DecodeScale + DecodeBias, (float3x3)WorldToViewMatrix ) )
//    * Using bent normals as input may result in false-occlusion (overdarkening) artifacts.
//      Such artifacts may be alleviated by increasing the AO Bias parameter.
//---------------------------------------------------------------------------------------------------

struct GFSDK_SSAO_InputNormalData
{
    GFSDK_SSAO_BOOL                 Enable;                         // To use the provided normals (instead of reconstructed ones)
    GFSDK_SSAO_Matrix               WorldToViewMatrix;              // 4x4 WorldToView matrix from the depth generation pass
    GFSDK_SSAO_FLOAT                DecodeScale;                    // Optional pre-matrix scale
    GFSDK_SSAO_FLOAT                DecodeBias;                     // Optional pre-matrix bias

    GFSDK_SSAO_InputNormalData()
        : Enable(false)
        , DecodeScale(1.f)
        , DecodeBias(0.f)
    {
    }
};

struct GFSDK_SSAO_InputNormalData_D3D12 : GFSDK_SSAO_InputNormalData
{
    GFSDK_SSAO_ShaderResourceView_D3D12 FullResNormalTextureSRV;    // Full-resolution world-space normal texture
};

struct GFSDK_SSAO_InputNormalData_D3D11 : GFSDK_SSAO_InputNormalData
{
    ID3D11ShaderResourceView*   pFullResNormalTextureSRV;           // Full-resolution world-space normal texture

    GFSDK_SSAO_InputNormalData_D3D11()
        : pFullResNormalTextureSRV(NULL)
    {
    }
};

//---------------------------------------------------------------------------------------------------
// Input data.
//---------------------------------------------------------------------------------------------------

struct GFSDK_SSAO_InputData_D3D12
{
    GFSDK_SSAO_InputDepthData_D3D12         DepthData;          // Required
    GFSDK_SSAO_InputNormalData_D3D12        NormalData;         // Optional GBuffer normals
};

struct GFSDK_SSAO_InputData_D3D11
{
    GFSDK_SSAO_InputDepthData_D3D11         DepthData;          // Required
    GFSDK_SSAO_InputNormalData_D3D11        NormalData;         // Optional GBuffer normals
};

/*====================================================================================================
   Parameters.
====================================================================================================*/

//---------------------------------------------------------------------------------------------------
// When enabled, the screen-space AO kernel radius is:
// - inversely proportional to ViewDepth for ViewDepth > ForegroundViewDepth
// - uniform in screen-space for ViewDepth <= ForegroundViewDepth
//---------------------------------------------------------------------------------------------------
struct GFSDK_SSAO_ForegroundAO
{
    GFSDK_SSAO_BOOL Enable;                                     // Enabling this may have a small performance impact
    GFSDK_SSAO_FLOAT ForegroundViewDepth;                       // View-space depth at which the AO footprint should get clamped

    GFSDK_SSAO_ForegroundAO()
        : Enable(false)
        , ForegroundViewDepth(0.f)
    {
    }
};

//---------------------------------------------------------------------------------------------------
// When enabled, the screen-space AO kernel radius is:
// - inversely proportional to ViewDepth for ViewDepth < BackgroundViewDepth
// - uniform in screen-space for ViewDepth >= BackgroundViewDepth (instead of falling off to zero)
//---------------------------------------------------------------------------------------------------
struct GFSDK_SSAO_BackgroundAO
{
    GFSDK_SSAO_BOOL Enable;                                     // Enabling this may have a small performance impact
    GFSDK_SSAO_FLOAT BackgroundViewDepth;                       // View-space depth at which the AO footprint should stop falling off with depth

    GFSDK_SSAO_BackgroundAO()
        : Enable(false)
        , BackgroundViewDepth(0.f)
    {
    }
};

struct GFSDK_SSAO_DepthThreshold
{
    GFSDK_SSAO_BOOL                 Enable;                     // To return white AO for ViewDepths > MaxViewDepth
    GFSDK_SSAO_FLOAT                MaxViewDepth;               // Custom view-depth threshold
    GFSDK_SSAO_FLOAT                Sharpness;                  // The higher, the sharper are the AO-to-white transitions

    GFSDK_SSAO_DepthThreshold()
        : Enable(false)
        , MaxViewDepth(0.f)
        , Sharpness(100.f)
    {
    }
};

//---------------------------------------------------------------------------------------------------
// When enabled, the actual per-pixel blur sharpness value depends on the per-pixel view depth with:
//     LerpFactor = (PixelViewDepth - ForegroundViewDepth) / (BackgroundViewDepth - ForegroundViewDepth)
//     Sharpness = lerp(Sharpness*ForegroundSharpnessScale, Sharpness, saturate(LerpFactor))
//---------------------------------------------------------------------------------------------------
struct GFSDK_SSAO_BlurSharpnessProfile
{
    GFSDK_SSAO_BOOL                 Enable;                         // To make the blur sharper in the foreground
    GFSDK_SSAO_FLOAT                ForegroundSharpnessScale;       // Sharpness scale factor for ViewDepths <= ForegroundViewDepth
    GFSDK_SSAO_FLOAT                ForegroundViewDepth;            // Maximum view depth of the foreground depth range
    GFSDK_SSAO_FLOAT                BackgroundViewDepth;            // Minimum view depth of the background depth range

    GFSDK_SSAO_BlurSharpnessProfile ()
        : Enable(false)
        , ForegroundSharpnessScale(4.f)
        , ForegroundViewDepth(0.f)
        , BackgroundViewDepth(1.f)
    {
    }
};

struct GFSDK_SSAO_BlurParameters
{
    GFSDK_SSAO_BOOL                 Enable;                     // To blur the AO with an edge-preserving blur
    GFSDK_SSAO_BlurRadius           Radius;                     // BLUR_RADIUS_2 or BLUR_RADIUS_4
    GFSDK_SSAO_FLOAT                Sharpness;                  // The higher, the more the blur preserves edges // 0.0~16.0
    GFSDK_SSAO_BlurSharpnessProfile SharpnessProfile;           // Optional depth-dependent sharpness function

    GFSDK_SSAO_BlurParameters()
        : Enable(true)
        , Radius(GFSDK_SSAO_BLUR_RADIUS_4)
        , Sharpness(16.f)
    {
    }
};

//---------------------------------------------------------------------------------------------------
// Remarks:
//    * The final occlusion is a weighted sum of 2 occlusion contributions. The SmallScaleAO and LargeScaleAO parameters are the weights.
//    * Setting the DepthStorage parameter to FP16_VIEW_DEPTHS is fastest but may introduce minor false-occlusion artifacts for large depths.
//---------------------------------------------------------------------------------------------------
struct GFSDK_SSAO_Parameters
{
    GFSDK_SSAO_FLOAT                Radius;                     // The AO radius in meters
    GFSDK_SSAO_FLOAT                Bias;                       // To hide low-tessellation artifacts // 0.0~0.5
    GFSDK_SSAO_FLOAT                SmallScaleAO;               // Scale factor for the small-scale AO, the greater the darker // 0.0~2.0
    GFSDK_SSAO_FLOAT                LargeScaleAO;               // Scale factor for the large-scale AO, the greater the darker // 0.0~2.0
    GFSDK_SSAO_FLOAT                PowerExponent;              // The final AO output is pow(AO, powerExponent) // 1.0~4.0
    GFSDK_SSAO_ForegroundAO         ForegroundAO;               // To limit the occlusion scale in the foreground
    GFSDK_SSAO_BackgroundAO         BackgroundAO;               // To add larger-scale occlusion in the distance
    GFSDK_SSAO_StepCount            StepCount;                  // The number of steps per direction in the AO-generation pass
    GFSDK_SSAO_DepthStorage         DepthStorage;               // The format of the internal depth texture sampled in the AO-generation pass
    GFSDK_SSAO_DepthClampMode       DepthClampMode;             // To hide possible false-occlusion artifacts near screen borders
    GFSDK_SSAO_DepthThreshold       DepthThreshold;             // Optional Z threshold, to hide possible depth-precision artifacts
    GFSDK_SSAO_BlurParameters       Blur;                       // Optional AO blur, to blur the AO before compositing it
    GFSDK_SSAO_BOOL                 EnableDualLayerAO;          // To reduce halo artifacts behind foreground object

    GFSDK_SSAO_Parameters()
        : Radius(1.f)
        , Bias(0.1f)
        , SmallScaleAO(1.f)
        , LargeScaleAO(1.f)
        , PowerExponent(2.f)
        , StepCount(GFSDK_SSAO_STEP_COUNT_4)
        , DepthStorage(GFSDK_SSAO_FP16_VIEW_DEPTHS)
        , DepthClampMode(GFSDK_SSAO_CLAMP_TO_EDGE)
        , EnableDualLayerAO(false)
    {
    }
};

/*====================================================================================================
   Output.
====================================================================================================*/

//---------------------------------------------------------------------------------------------------
// [Optional] Custom blend state.
//---------------------------------------------------------------------------------------------------

struct GFSDK_SSAO_CustomBlendState_D3D12
{
    struct D3D12_BLEND_DESC*        pBlendState;
    const GFSDK_SSAO_FLOAT*         pBlendFactor;

    GFSDK_SSAO_CustomBlendState_D3D12()
        : pBlendState(NULL)
        , pBlendFactor(NULL)
    {
    }
};

struct GFSDK_SSAO_CustomBlendState_D3D11
{
    ID3D11BlendState*               pBlendState;
    const GFSDK_SSAO_FLOAT*         pBlendFactor;

    GFSDK_SSAO_CustomBlendState_D3D11()
        : pBlendState(NULL)
        , pBlendFactor(NULL)
    {
    }
};

//---------------------------------------------------------------------------------------------------
// Compositing blend state.
//---------------------------------------------------------------------------------------------------

struct GFSDK_SSAO_BlendState_D3D12
{
    GFSDK_SSAO_BlendMode                Mode;                   // OVERWRITE_RGB, MULTIPLY_RGB or CUSTOM_BLEND
    GFSDK_SSAO_CustomBlendState_D3D12   CustomState;            // Relevant only if Mode is CUSTOM_BLEND

    GFSDK_SSAO_BlendState_D3D12()
        : Mode(GFSDK_SSAO_OVERWRITE_RGB)
    {
    }
};

struct GFSDK_SSAO_BlendState_D3D11
{
    GFSDK_SSAO_BlendMode                Mode;                   // OVERWRITE_RGB, MULTIPLY_RGB or CUSTOM_BLEND
    GFSDK_SSAO_CustomBlendState_D3D11   CustomState;            // Relevant only if Mode is CUSTOM_BLEND

    GFSDK_SSAO_BlendState_D3D11()
        : Mode(GFSDK_SSAO_OVERWRITE_RGB)
    {
    }
};

//---------------------------------------------------------------------------------------------------
// [Optional] Custom depth-stencil state.
//---------------------------------------------------------------------------------------------------

struct GFSDK_SSAO_CustomDepthStencilState_D3D11
{
    ID3D11DepthStencilState*        pDepthStencilState;
    GFSDK_SSAO_UINT                 StencilRef;

    GFSDK_SSAO_CustomDepthStencilState_D3D11()
        : pDepthStencilState(NULL)
        , StencilRef(0)
    {
    }
};

struct GFSDK_SSAO_DepthStencilState_D3D11
{
    GFSDK_SSAO_DepthStencilMode                 Mode;               // DISABLED_DEPTH_STENCIL or CUSTOM_DEPTH_STENCIL
    GFSDK_SSAO_CustomDepthStencilState_D3D11    CustomState;        // Relevant only if Mode is CUSTOM_DEPTH_STENCIL

    GFSDK_SSAO_DepthStencilState_D3D11()
        : Mode(GFSDK_SSAO_DISABLED_DEPTH_STENCIL)
    {
    }
};

//---------------------------------------------------------------------------------------------------
// Output render target & compositing state.
//---------------------------------------------------------------------------------------------------

struct GFSDK_SSAO_Output_D3D12
{
    GFSDK_SSAO_RenderTargetView_D3D12*  pRenderTargetView;      // Output render target of RenderAO
    GFSDK_SSAO_BlendState_D3D12         Blend;                  // Blend state used when writing the AO to pRenderTargetView

    GFSDK_SSAO_Output_D3D12()
        : pRenderTargetView(NULL)
    {
    }
};

struct GFSDK_SSAO_Output_D3D11
{
    ID3D11RenderTargetView*             pRenderTargetView;      // Output render target of RenderAO
    GFSDK_SSAO_BlendState_D3D11         Blend;                  // Blend state used when writing the AO to pRenderTargetView

    GFSDK_SSAO_Output_D3D11()
        : pRenderTargetView(NULL)
    {
    }
};

/*====================================================================================================
  [Optional] Let the library allocate its memory on a custom heap.
====================================================================================================*/

struct GFSDK_SSAO_CustomHeap
{
    GFSDK_SSAO_CustomHeap()
        : new_(NULL)
        , delete_(NULL)
    {
    }
    void* (*new_)(size_t);
    void (*delete_)(void*);
};

/*====================================================================================================
  [Optional] For debugging any issues with the input projection matrix.
====================================================================================================*/

struct GFSDK_SSAO_ProjectionMatrixDepthRange
{
    GFSDK_SSAO_ProjectionMatrixDepthRange()
        : ZNear(-1.f)
        , ZFar(-1.f)
    {
    }
    GFSDK_SSAO_FLOAT ZNear;
    GFSDK_SSAO_FLOAT ZFar;
};

/*====================================================================================================
   Base interface.
====================================================================================================*/

class GFSDK_SSAO_Context
{
public:

//---------------------------------------------------------------------------------------------------
// [Optional] Returns the amount of video memory allocated by the library, in bytes.
//---------------------------------------------------------------------------------------------------
virtual GFSDK_SSAO_UINT GetAllocatedVideoMemoryBytes() = 0;

}; //class GFSDK_SSAO_Context

//---------------------------------------------------------------------------------------------------
// [Optional] Returns the DLL version information.
//---------------------------------------------------------------------------------------------------
GFSDK_SSAO_DECL(GFSDK_SSAO_Status, GFSDK_SSAO_GetVersion, GFSDK_SSAO_Version* pVersion);

/*====================================================================================================
   D3D11 interface.
====================================================================================================*/

//---------------------------------------------------------------------------------------------------
// Note: The RenderAO, PreCreateRTs and Release entry points should not be called simultaneously from different threads.
//---------------------------------------------------------------------------------------------------
class GFSDK_SSAO_Context_D3D11 : public GFSDK_SSAO_Context
{
public:

    //---------------------------------------------------------------------------------------------------
    // Renders SSAO.
    //
    // Remarks:
    //    * Allocates internal D3D render targets on first use, and re-allocates them when the viewport dimensions change.
    //    * All the relevant device-context states are saved and restored internally when entering and exiting the call.
    //    * Setting RenderMask = GFSDK_SSAO_RENDER_DEBUG_NORMAL_Z can be useful to visualize the normals used for the AO rendering.
    //
    // Returns:
    //     GFSDK_SSAO_NULL_ARGUMENT                        - One of the required argument pointers is NULL
    //     GFSDK_SSAO_INVALID_PROJECTION_MATRIX            - The projection matrix is not valid
    //     GFSDK_SSAO_INVALID_VIEWPORT_DIMENSIONS          - One of the viewport dimensions (width or height) is 0
    //     GFSDK_SSAO_INVALID_VIEWPORT_DEPTH_RANGE         - The viewport depth range is not a sub-range of [0.f,1.f]
    //     GFSDK_SSAO_INVALID_WORLD_TO_VIEW_MATRIX         - The world-to-view matrix is not valid (transposing it may help)
    //     GFSDK_SSAO_INVALID_NORMAL_TEXTURE_RESOLUTION    - The normal-texture resolution does not match the depth-texture resolution
    //     GFSDK_SSAO_INVALID_NORMAL_TEXTURE_SAMPLE_COUNT  - The normal-texture sample count does not match the depth-texture sample count
    //     GFSDK_SSAO_D3D_RESOURCE_CREATION_FAILED         - A D3D resource-creation call has failed (running out of memory?)
    //     GFSDK_SSAO_OK                                   - Success
    //---------------------------------------------------------------------------------------------------
    virtual GFSDK_SSAO_Status RenderAO(
        ID3D11DeviceContext* pDeviceContext,
        const GFSDK_SSAO_InputData_D3D11& InputData,
        const GFSDK_SSAO_Parameters& Parameters,
        const GFSDK_SSAO_Output_D3D11& Output,
        GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_AO) = 0;

    //---------------------------------------------------------------------------------------------------
    // [Optional] Pre-creates all internal render targets for RenderAO.
    //
    // Remarks:
    //    * This call may be safely skipped since RenderAO creates its render targets on demand if they were not pre-created.
    //    * This call releases and re-creates the internal render targets if the provided resolution changes.
    //    * This call performs CreateTexture calls for all the relevant render targets.
    //
    // Returns:
    //     GFSDK_SSAO_NULL_ARGUMENT                        - One of the required argument pointers is NULL
    //     GFSDK_SSAO_D3D_RESOURCE_CREATION_FAILED         - A D3D resource-creation call has failed (running out of memory?)
    //     GFSDK_SSAO_OK                                   - Success
    //---------------------------------------------------------------------------------------------------
    virtual GFSDK_SSAO_Status PreCreateRTs(
        const GFSDK_SSAO_Parameters& Parameters,
        GFSDK_SSAO_UINT ViewportWidth,
        GFSDK_SSAO_UINT ViewportHeight) = 0;

    //---------------------------------------------------------------------------------------------------
    // [Optional] Gets the library-internal ZNear and ZFar values derived from the input projection matrix.
    //
    // Remarks:
    //    * HBAO+ supports all perspective projection matrices, with arbitrary ZNear and ZFar.
    //    * For reverse infinite projections, GetProjectionMatrixDepthRange should return ZNear=+INF and ZFar=0.f.
    //
    // Returns:
    //     GFSDK_SSAO_NULL_ARGUMENT                        - One of the required argument pointers is NULL
    //     GFSDK_SSAO_INVALID_PROJECTION_MATRIX            - The projection matrix is not valid
    //     GFSDK_SSAO_OK                                   - Success
    //---------------------------------------------------------------------------------------------------
    virtual GFSDK_SSAO_Status GetProjectionMatrixDepthRange(
        const GFSDK_SSAO_InputData_D3D11& InputData,
        GFSDK_SSAO_ProjectionMatrixDepthRange& OutputDepthRange) = 0;

    //---------------------------------------------------------------------------------------------------
    // Releases all D3D objects created by the library (to be called right before releasing the D3D device).
    //---------------------------------------------------------------------------------------------------
    virtual void Release() = 0;

}; //class GFSDK_SSAO_Context_D3D11

//---------------------------------------------------------------------------------------------------
// Creates a GFSDK_SSAO_Context associated with the D3D11 device.
//
// Remarks:
//    * Allocates D3D11 resources internally.
//    * Allocates memory using the default "::operator new", or "pCustomHeap->new_" if provided.
//
// Returns:
//     GFSDK_SSAO_NULL_ARGUMENT                        - One of the required argument pointers is NULL
//     GFSDK_SSAO_VERSION_MISMATCH                     - Invalid HeaderVersion (have you set HeaderVersion = GFSDK_SSAO_Version()?)
//     GFSDK_SSAO_MEMORY_ALLOCATION_FAILED             - Failed to allocate memory on the heap
//     GFSDK_SSAO_D3D_FEATURE_LEVEL_NOT_SUPPORTED      - The D3D feature level of pD3DDevice is lower than 11_0
//     GFSDK_SSAO_D3D_RESOURCE_CREATION_FAILED         - A resource-creation call has failed (running out of memory?)
//     GFSDK_SSAO_OK                                   - Success
//---------------------------------------------------------------------------------------------------
GFSDK_SSAO_DECL(GFSDK_SSAO_Status, GFSDK_SSAO_CreateContext_D3D11,
    ID3D11Device* pD3DDevice,
    GFSDK_SSAO_Context_D3D11** ppContext,
    GFSDK_SSAO_CUSTOM_HEAP_ARGUMENT,
    GFSDK_SSAO_VERSION_ARGUMENT);

/*====================================================================================================
   D3D12 interface.
====================================================================================================*/

//---------------------------------------------------------------------------------------------------
// Note: The RenderAO, PreCreateRTs and Release entry points should not be called simultaneously from different threads.
//---------------------------------------------------------------------------------------------------
class GFSDK_SSAO_Context_D3D12 : public GFSDK_SSAO_Context
{
public:

    //---------------------------------------------------------------------------------------------------
    // Renders SSAO.
    //
    // Remarks:
    //    * Allocates internal D3D render targets on first use, and re-allocates them when the viewport dimensions change.
    //    * Setting RenderMask = GFSDK_SSAO_RENDER_DEBUG_NORMAL_Z can be useful to visualize the normals used for the AO rendering.
    //    * The input depth & normal textures are assumed to have state D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE.
    //    * The output AO render target is assumed to have state D3D12_RESOURCE_STATE_RENDER_TARGET.
    //
    // Returns:
    //     GFSDK_SSAO_NULL_ARGUMENT                        - One of the required argument pointers is NULL
    //     GFSDK_SSAO_INVALID_PROJECTION_MATRIX            - The projection matrix is not valid
    //     GFSDK_SSAO_INVALID_VIEWPORT_DIMENSIONS          - One of the viewport dimensions (width or height) is 0
    //     GFSDK_SSAO_INVALID_VIEWPORT_DEPTH_RANGE         - The viewport depth range is not a sub-range of [0.f,1.f]
    //     GFSDK_SSAO_INVALID_WORLD_TO_VIEW_MATRIX         - The world-to-view matrix is not valid (transposing it may help)
    //     GFSDK_SSAO_INVALID_NORMAL_TEXTURE_RESOLUTION    - The normal-texture resolution does not match the depth-texture resolution
    //     GFSDK_SSAO_INVALID_NORMAL_TEXTURE_SAMPLE_COUNT  - The normal-texture sample count does not match the depth-texture sample count
    //     GFSDK_SSAO_D3D_RESOURCE_CREATION_FAILED         - A D3D resource-creation call has failed (running out of memory?)
    //     GFSDK_SSAO_OK                                   - Success
    //---------------------------------------------------------------------------------------------------
    virtual GFSDK_SSAO_Status RenderAO(
        ID3D12CommandQueue* pCmdQueue,
        ID3D12GraphicsCommandList* pCmdList,
        const GFSDK_SSAO_InputData_D3D12& InputData,
        const GFSDK_SSAO_Parameters& Parameters,
        const GFSDK_SSAO_Output_D3D12& Output,
        GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_AO) = 0;

    //---------------------------------------------------------------------------------------------------
    // [Optional] Pre-creates all internal render targets for RenderAO.
    //
    // Remarks:
    //    * This call may be safely skipped since RenderAO creates its render targets on demand if they were not pre-created.
    //    * This call releases and re-creates the internal render targets if the provided resolution changes.
    //    * This call performs CreateCommittedResource calls for all the relevant render targets.
    //
    // Returns:
    //     GFSDK_SSAO_NULL_ARGUMENT                        - One of the required argument pointers is NULL
    //     GFSDK_SSAO_D3D_RESOURCE_CREATION_FAILED         - A D3D resource-creation call has failed (running out of memory?)
    //     GFSDK_SSAO_OK                                   - Success
    //---------------------------------------------------------------------------------------------------
    virtual GFSDK_SSAO_Status PreCreateRTs(
        ID3D12CommandQueue* pCmdQueue,
        const GFSDK_SSAO_Parameters& Parameters,
        GFSDK_SSAO_UINT ViewportWidth,
        GFSDK_SSAO_UINT ViewportHeight) = 0;

    //---------------------------------------------------------------------------------------------------
    // [Optional] Gets the library-internal ZNear and ZFar values derived from the input projection matrix.
    //
    // Remarks:
    //    * HBAO+ supports all perspective projection matrices, with arbitrary ZNear and ZFar.
    //    * For reverse infinite projections, GetProjectionMatrixDepthRange should return ZNear=+INF and ZFar=0.f.
    //
    // Returns:
    //     GFSDK_SSAO_NULL_ARGUMENT                        - One of the required argument pointers is NULL
    //     GFSDK_SSAO_INVALID_PROJECTION_MATRIX            - The projection matrix is not valid
    //     GFSDK_SSAO_OK                                   - Success
    //---------------------------------------------------------------------------------------------------
    virtual GFSDK_SSAO_Status GetProjectionMatrixDepthRange(
        const GFSDK_SSAO_InputData_D3D12& InputData,
        GFSDK_SSAO_ProjectionMatrixDepthRange& OutputDepthRange) = 0;

    //---------------------------------------------------------------------------------------------------
    // Releases all D3D objects created by the library (to be called right before releasing the D3D device).
    //---------------------------------------------------------------------------------------------------
    virtual void Release() = 0;

}; //class GFSDK_SSAO_Context_D3D12

//---------------------------------------------------------------------------------------------------
//
// Remark:
//    * Describes the D3D12 descriptor heap space that is passed to the library
//
//---------------------------------------------------------------------------------------------------
#define GFSDK_SSAO_NUM_DESCRIPTORS_RTV_HEAP_D3D12           40  // Number of required descriptors for RTV heap type
#define GFSDK_SSAO_NUM_DESCRIPTORS_CBV_SRV_UAV_HEAP_D3D12   60  // Number of required descriptors for CBV/SRV/UAV heap type

struct GFSDK_SSAO_DescriptorHeapRange_D3D12
{
    ID3D12DescriptorHeap* pDescHeap;           // Pointer to the descriptor heap
    GFSDK_SSAO_UINT       BaseIndex;           // The base index that can be used as a start index of the pDescHeap

    GFSDK_SSAO_DescriptorHeapRange_D3D12()
        : pDescHeap(NULL)
        , BaseIndex(0)
    {
    }
};

struct GFSDK_SSAO_DescriptorHeaps_D3D12
{
    GFSDK_SSAO_DescriptorHeapRange_D3D12 CBV_SRV_UAV;
    GFSDK_SSAO_DescriptorHeapRange_D3D12 RTV;
};

//---------------------------------------------------------------------------------------------------
// Creates a GFSDK_SSAO_Context associated with the D3D12 device.
//
// Remarks:
//    * Allocates D3D12 resources internally.
//    * Allocates memory using the default "::operator new", or "pCustomHeap->new_" if provided.
//
// Returns:
//     GFSDK_SSAO_NULL_ARGUMENT                        - One of the required argument pointers is NULL
//     GFSDK_SSAO_VERSION_MISMATCH                     - Invalid HeaderVersion (have you set HeaderVersion = GFSDK_SSAO_Version()?)
//     GFSDK_SSAO_MEMORY_ALLOCATION_FAILED             - Failed to allocate memory on the heap
//     GFSDK_SSAO_D3D_FEATURE_LEVEL_NOT_SUPPORTED      - The D3D feature level of pD3DDevice is lower than 11_0
//     GFSDK_SSAO_D3D_RESOURCE_CREATION_FAILED         - A resource-creation call has failed (running out of memory?)
//     GFSDK_SSAO_D3D12_INVALID_HEAP_TYPE              - One of the heaps provided to GFSDK_SSAO_CreateContext_D3D12 has an unexpected type
//     GFSDK_SSAO_D3D12_INSUFFICIENT_DESCRIPTORS       - One of the heaps described in pHeapInfo has an insufficient number of descriptors
//     GFSDK_SSAO_D3D12_INVALID_NODE_MASK              - NodeMask has more than one bit set, or is zero
//     GFSDK_SSAO_OK                                   - Success
//---------------------------------------------------------------------------------------------------
GFSDK_SSAO_DECL(GFSDK_SSAO_Status, GFSDK_SSAO_CreateContext_D3D12,
    ID3D12Device* pD3DDevice,
    GFSDK_SSAO_UINT NodeMask,
    const GFSDK_SSAO_DescriptorHeaps_D3D12& DescriptorHeaps,
    GFSDK_SSAO_Context_D3D12** ppContext,
    GFSDK_SSAO_CUSTOM_HEAP_ARGUMENT,
    GFSDK_SSAO_VERSION_ARGUMENT);

#pragma pack(pop)
