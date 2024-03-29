﻿# HBAOPlus

Welcome to NVIDIA's HBAO+ source code repository.
To begin, clone this repository onto your local drive.

See Documentation under \doc.

NVIDIA HBAO+ 4.0.
----------------------

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

Package
--------
doc/—HTML documentation

lib/—header file, import libraries and DLLs, for Win32 and Win64.

samples/—source for DX11 & DX12 sample applications demonstrating NVIDIA HBAO+.

Getting Started
---------------

INITIALIZE THE LIBRARY:
```
GFSDK_SSAO_CustomHeap CustomHeap;
CustomHeap.new_ = ::operator new;
CustomHeap.delete_ = ::operator delete;

GFSDK_SSAO_Status status;
GFSDK_SSAO_Context_D3D11* pAOContext;
status = GFSDK_SSAO_CreateContext_D3D11(pD3D11Device, &pAOContext, &CustomHeap);
assert(status == GFSDK_SSAO_OK); // HBAO+ requires feature level 11_0 or above
```

SET INPUT DEPTHS:
```
GFSDK_SSAO_InputData_D3D11 Input;
Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
Input.DepthData.pFullResDepthTextureSRV = pDepthStencilTextureSRV;
Input.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4(pProjectionMatrix);
Input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
Input.DepthData.MetersToViewSpaceUnits = SceneScale;
```

SET AO PARAMETERS:
```
GFSDK_SSAO_Parameters Params;
Params.Radius = 2.f;
Params.Bias = 0.1f;
Params.PowerExponent = 2.f;
Params.Blur.Enable = true;
Params.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
Params.Blur.Sharpness = 16.f;
```

SET RENDER TARGET:
```
GFSDK_SSAO_Output_D3D11 Output;
Output.pRenderTargetView = pOutputColorRTV;
Output.Blend.Mode = GFSDK_SSAO_OVERWRITE_RGB;
```

RENDER AO:
```
status = pAOContext->RenderAO(pD3D11Context, Input, Params, Output);
assert(status == GFSDK_SSAO_OK);
```

Data Flow
---------
Input Requirements
The library has entry points for DX11 and DX12.
Requires a depth texture to be provided as input, along with associated projection info.
Optionally, can also take as input a GBuffer normal texture associated with the input depth texture:
Can add normal-mapping details to the AO.
Can be used to fix normal reconstruction artifacts with dithered LOD dissolves.
But makes the integration more complex. We recommend starting with input normals disabled.
Optionally, can also take as input a viewport rectangle associated with the input textures:
Defines a sub-area of the input & output full-resolution textures to be sourced and rendered to.
The library re-allocates its internal render targets if the Viewport.Width or Viewport.Height changes for a given AO context.

MSAA Support
------------
Requirements
The input depth & normal textures are required to have matching dimensions and MSAA sample count.
The output render target can have arbitrary dimensions and MSAA sample count.
Per-Pixel AO Algorithm
If the input textures are MSAA, only sample 0 is used to render the AO.
If the output render target is MSAA, a per-pixel AO value is written to all samples.
In practice, we have found this strategy to not cause any objectionable artifacts, even when using HBAO+ with TXAA.
