/*
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include <DXUT.h>
#include <DXUTgui.h>
#include <DXUTmisc.h>
#include <DXUTCamera.h>
#include <DXUTSettingsDlg.h>
#include <SDKmisc.h>

#include "..\..\..\src\Renderer_DX11.h"
#include "SceneRTs.h"
#include "Scene3D.h"
#include "GPUProfiler.h"

#define ENABLE_MESHES 1
#define ENABLE_SHOW_COLORS 0
#define ENABLE_GBUFFER_NORMALS 1
#define ENABLE_DEBUG_NORMALS 0
#define ENABLE_SMALL_SCALE_AO 1
#define ENABLE_LARGE_SCALE_AO 1
#define ENABLE_BLUR_RADIUS 1
#define ENABLE_BORDER_PIXELS 0
#define ENABLE_FP16_VIEW_DEPTHS 1
#define ENABLE_DEPTH_CLAMP_MODES 0
#define ENABLE_BLUR_SHARPNESS 1
#define ENABLE_BLUR_SHARPNESS_PROFILE 0
#define ENABLE_BACKGROUND_AO 0
#define ENABLE_FOREGROUND_AO 0
#define ENABLE_VIEW_DEPTH_THRESHOLD 0
#define ENABLE_INTERNAL_RENDER_TIMES 1
#define ENABLE_HUD_BACKGROUND 0
#define ENABLE_NVAPI 0

#define TEST_TWO_PASS_BLEND 1
#define TEST_PARTIAL_VIEWPORT_DIMENSIONS 0
#define TEST_PARTIAL_VIEWPORT_DEPTH_RANGE 0
#define TEST_PRE_CREATE_RTS 1
#define TEST_REVERSED_DEPTH_TEST 1

//--------------------------------------------------------------------------------------
// Scene description
//--------------------------------------------------------------------------------------

#define FOVY (40.0f * D3DX_PI / 180.0f)
#define ZNEAR 0.01f
#define ZFAR 500.0f

#define MESH_PATH       L"..\\assets\\"

MeshDescriptor g_MeshDesc[] =
{
    //{L"Sibenik",   MESH_PATH L"sibenik.sdkmesh", SCENE_NO_GROUND_PLANE,  SCENE_NO_SHADING,  SCENE_USE_FIRST_PERSON_CAMERA, 0.005f },
    {L"AT-AT",     MESH_PATH L"AT-AT.sdkmesh",   SCENE_USE_GROUND_PLANE, SCENE_NO_SHADING,  SCENE_USE_ORBITAL_CAMERA, 0.1f },
};

//--------------------------------------------------------------------------------------
// MSAA settings
//--------------------------------------------------------------------------------------

struct MSAADescriptor
{
    WCHAR Name[32];
    int SampleCount;
};

MSAADescriptor g_MSAADesc[] =
{
    {L"1x MSAA",   1},
    {L"2x MSAA",   2},
    {L"4x MSAA",   4},
    {L"8x MSAA",   8},
    {L"",          0},
};

//--------------------------------------------------------------------------------------
// GUI constants
//--------------------------------------------------------------------------------------

#define MAX_RADIUS_MULT 2.f
#define MAX_BACKGROUND_DEPTH 800.f
#define MAX_DEPTH_THRESHOLD 1.f
#define MAX_DEPTH_SHARPNESS 32.f
#define MAX_BLUR_SHARPNESS 32.f

enum
{
    IDC_TOGGLEFULLSCREEN = 1,
    IDC_TOGGLEREF,
    IDC_CHANGEDEVICE,
    IDC_CHANGESCENE,
    IDC_CHANGEMSAA,
#if ENABLE_SHOW_COLORS
    IDC_SHOW_COLORS,
    IDC_SHOW_AO,
#endif
#if ENABLE_GBUFFER_NORMALS
    IDC_USE_GBUFFER_NORMALS,
#endif
#if ENABLE_DEBUG_NORMALS
    IDC_DEBUG_NORMALS,
#endif
    IDC_RADIUS_STATIC,
    IDC_RADIUS_SLIDER,
#if ENABLE_BACKGROUND_AO
    IDC_BACKGROUND_DEPTH_STATIC,
    IDC_BACKGROUND_DEPTH_SLIDER,
#endif
#if ENABLE_FOREGROUND_AO
    IDC_FOREGROUND_DEPTH_STATIC,
    IDC_FOREGROUND_DEPTH_SLIDER,
#endif
#if ENABLE_VIEW_DEPTH_THRESHOLD
    IDC_DEPTH_THRESHOLD_STATIC,
    IDC_DEPTH_THRESHOLD_SLIDER,
    IDC_DEPTH_SHARPNESS_STATIC,
    IDC_DEPTH_SHARPNESS_SLIDER,
#endif
    IDC_BIAS_STATIC,
    IDC_BIAS_SLIDER,
    IDC_SMALL_SCALE_AO_STATIC,
    IDC_SMALL_SCALE_AO_SLIDER,
#if ENABLE_LARGE_SCALE_AO
    IDC_LARGE_SCALE_AO_STATIC,
    IDC_LARGE_SCALE_AO_SLIDER,
#endif
    IDC_EXPONENT_STATIC,
    IDC_EXPONENT_SLIDER,
#if ENABLE_BLUR_RADIUS
    IDC_BLUR_DISABLED,
    IDC_BLUR_RADIUS_2,
    IDC_BLUR_RADIUS_4,
#endif
#if ENABLE_BORDER_PIXELS
    IDC_BORDER_PIXELS_0,
    IDC_BORDER_PIXELS_64,
    IDC_BORDER_PIXELS_128,
#endif
#if ENABLE_BLUR_SHARPNESS
    IDC_BLUR_SHARPNESS_STATIC,
    IDC_BLUR_SHARPNESS_SLIDER,
#endif
#if ENABLE_FP16_VIEW_DEPTHS
    IDC_DEPTH_STORAGE_TYPE_FP16,
    IDC_DEPTH_STORAGE_TYPE_FP32,
#endif
#if ENABLE_DEPTH_CLAMP_MODES
    IDC_DEPTH_CLAMP_TO_EDGE,
    IDC_DEPTH_CLAMP_TO_BORDER,
#endif
#if ENABLE_DEBUG_MODES
    IDC_SHADER_TYPE_HBAO_PLUS,
    IDC_SHADER_TYPE_DEBUG_HBAO_PLUS,
#endif
};

enum
{
    MSAA_MODE_1X = 0,
    MSAA_MODE_2X,
    MSAA_MODE_4X,
    MSAA_MODE_8X,
};

#define UI_RADIUS_MULT      L"Radius multiplier: "
#define UI_BACKGROUND_DEPTH L"Background depth:"
#define UI_FOREGROUND_DEPTH L"Foreground depth:"
#define UI_DEPTH_THRESHOLD  L"Depth threshold: "
#define UI_DEPTH_SHARPNESS  L"Depth sharpness: "
#define UI_AO_BIAS          L"Bias: "
#define UI_POW_EXPONENT     L"Power exponent: "
#define UI_SMALL_SCALE_AO   L"Small-scale AO: "
#define UI_LARGE_SCALE_AO   L"Large-scale AO: "
#define UI_BLUR_SHARPNESS   L"Blur sharpness: "

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

CDXUTDialogResourceManager    g_DialogResourceManager;  // manager for shared resources of dialogs
CD3DSettingsDlg               g_SettingsDlg;            // Device settings dialog
CDXUTDialog                   g_HUD;                    // dialog for standard controls
CDXUTTextHelper*              g_pTxtHelper = NULL;
UINT                          g_TextLineHeight = 15;
bool                          g_DrawUI = true;
bool                          g_ShowAO = true;
bool                          g_BlurAO = true;
bool                          g_ShowColors = false;
bool                          g_PerSampleAO = true;
bool                          g_UseGBufferNormals = true;
bool                          g_DebugNormals = false;
GFSDK_SSAO_RenderMask         g_RenderMask;
GFSDK_SSAO_Parameters         g_AOParams;
GFSDK_SSAO_Context_D3D11*     g_pAOContext = NULL;
GFSDK_SSAO_ProjectionMatrixDepthRange g_ProjectionMatrixDepthRange;

D3D11_VIEWPORT                g_BackBufferViewport;
UINT                          g_BackBufferWidth = 0;
UINT                          g_BackBufferHeight = 0;
UINT                          g_BorderPixels = 0;
SceneRenderer                 g_pSceneRenderer;
SceneRTs*                     g_pSceneRTs = NULL;

#if TEST_TWO_PASS_BLEND
ID3D11BlendState*             g_pMinBlendState;
ID3D11DepthStencilState*      g_pStencilEqualState;
bool                          g_EnableTwoPassBlend = true;
#endif

#if TEST_PARTIAL_VIEWPORT_DIMENSIONS
BOOL                          g_TestSmallViewport = FALSE;
#endif

#if ENABLE_NVAPI
BOOL                          g_MonitorAvailableVidMem = FALSE;
BOOL                          g_MonitorNvGraphicsClock = TRUE;
#endif

#if ENABLE_RENDER_TIMES
GFSDK::SSAO::GpuTimeRenderPassIndex GFSDK::SSAO::RenderTimes::CurrentPassIndex = GFSDK::SSAO::RENDER_PASS_0;
#endif

SceneMesh g_SceneMeshes[SIZEOF_ARRAY(g_MeshDesc)];
D3D11::GPUProfiler g_GPUProfiler;

struct Scene
{
    Scene()
        : pMesh(NULL)
    {
    }
    SceneMesh *pMesh;
};
Scene g_Scenes[SIZEOF_ARRAY(g_MeshDesc)];
bool g_UseOrbitalCamera = true;

int g_CurrentSceneId = 0;
int g_MSAACurrentSettings = MSAA_MODE_1X;

#if 1
#define RENDER_AO_WIDTH     g_BackBufferWidth
#define RENDER_AO_HEIGHT    g_BackBufferHeight
#else
#define RENDER_AO_WIDTH     3840
#define RENDER_AO_HEIGHT    2160
#endif

//--------------------------------------------------------------------------------------
class CPUTimer
{
public:
    CPUTimer(float* pTimeUS)
    {
        QueryPerformanceCounter(&m_StartTime);
        m_pTimeUS = pTimeUS;
    }
    ~CPUTimer()
    {
        LARGE_INTEGER EndTime, Freq;
        QueryPerformanceCounter(&EndTime);
        QueryPerformanceFrequency(&Freq);
        *m_pTimeUS = float((((double)EndTime.QuadPart) - ((double)m_StartTime.QuadPart))/((double)Freq.QuadPart) * 1.e6);
    }

private:
    LARGE_INTEGER m_StartTime;
    float* m_pTimeUS;
};
float g_CPUTimeRenderAO = 0.f;
float g_CPUTimePreCreateRTs = 0.f;

//--------------------------------------------------------------------------------------
#if ENABLE_NVAPI
#include "NVAPI/nvapi.h"
//#pragma comment(lib, "nvapi")
class NvApiWrapper
{
public:
    struct MemInfo
    {
        unsigned int DedicatedVideoMemoryInMB;
        unsigned int AvailableDedicatedVideoMemoryInMB;
        unsigned int CurrentAvailableDedicatedVideoMemoryInMB;
    };
    struct FrequencyInfo
    {
        unsigned int NvGraphicsClockInMhz;
    };

    NvApiWrapper()
        : m_GpuHandle(0)
    {
    }

    void Init()
    {
        NvAPI_Status Status = NvAPI_Initialize();
        assert(Status == NVAPI_OK);

        NvPhysicalGpuHandle NvGpuHandles[NVAPI_MAX_PHYSICAL_GPUS] = { 0 };
        NvU32 NvGpuCount = 0;
        Status = NvAPI_EnumPhysicalGPUs(NvGpuHandles, &NvGpuCount);
        assert(Status == NVAPI_OK);
        assert(NvGpuCount != 0);
        m_GpuHandle = NvGpuHandles[0];
    }

    bool GetVideoMemoryInfo(MemInfo* pInfo)
    {
        NV_DISPLAY_DRIVER_MEMORY_INFO MemInfo = { 0 };
        MemInfo.version = NV_DISPLAY_DRIVER_MEMORY_INFO_VER_2;
        NvAPI_Status Status = NvAPI_GPU_GetMemoryInfo(m_GpuHandle, &MemInfo);
        if (Status != NVAPI_OK)
        {
            return false;
        }

        pInfo->DedicatedVideoMemoryInMB = MemInfo.dedicatedVideoMemory / 1024;
        pInfo->AvailableDedicatedVideoMemoryInMB = MemInfo.availableDedicatedVideoMemory / 1024;
        pInfo->CurrentAvailableDedicatedVideoMemoryInMB = MemInfo.curAvailableDedicatedVideoMemory / 1024;
        return true;
    }

    bool GetNVGraphicsClockMhz(FrequencyInfo *pInfo)
    {
        NV_GPU_CLOCK_FREQUENCIES table = { 0 };
        table.version = NV_GPU_CLOCK_FREQUENCIES_VER;
        table.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;

        NvAPI_Status Status = NvAPI_GPU_GetAllClockFrequencies(m_GpuHandle, &table);
        if (Status != NVAPI_OK)
        {
            return false;
        }

        if (!table.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent)
        {
            return false;
        }

        NvU32 GraphicsClockInKhz = table.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency;
        pInfo->NvGraphicsClockInMhz = NvU32((GraphicsClockInKhz + 500) / 1000);
        return true;
    }

private:
    NvPhysicalGpuHandle m_GpuHandle;
};
NvApiWrapper g_NvApiWrapper;
#endif

//--------------------------------------------------------------------------------------
using namespace GFSDK;

class TextRenderer
{
public:
    void Init(CDXUTDialogResourceManager* pManager)
    {
#if ENABLE_HUD_BACKGROUND
        const UINT NumTextLines = 5;
        const float HudOpacity = 0.32f;
        m_BackgroundQuad.Init(pManager);
        m_BackgroundQuad.SetLocation(0,0);
        m_BackgroundQuad.SetSize(260, g_TextLineHeight * NumTextLines + 15);
        m_BackgroundQuad.SetBackgroundColors(D3DCOLOR_COLORVALUE(1,1,1,HudOpacity));
#endif
#if ENABLE_NVAPI
        g_NvApiWrapper.Init();
#endif
    }

    void DrawText()
    {
        g_pTxtHelper->Begin();

        g_pTxtHelper->SetInsertionPos(5, 5);
        g_pTxtHelper->SetForegroundColor(D3DXCOLOR(0.f, 0.f, 0.f, 1.f));

        GFSDK_SSAO_Version Version;
        GFSDK_SSAO_Status Status;
        Status = GFSDK_SSAO_GetVersion(&Version);
        assert(Status == GFSDK_SSAO_OK);

        WCHAR VersionString[128];
        StringCchPrintf(VersionString, SIZEOF_ARRAY(VersionString), L"%d.%d.%d.%d", Version.Major, Version.Minor, Version.Branch, Version.Revision);
        g_pTxtHelper->DrawFormattedTextLine(L"D3D11 HBAO+ %s\n", VersionString);

        g_pTxtHelper->DrawTextLine(DXUTGetDeviceStats());

        UINT AOWidth = g_pSceneRTs->Width;
        UINT AOHeight = g_pSceneRTs->Height;
        g_pTxtHelper->DrawFormattedTextLine(L"AO Resolution: %d x %d", AOWidth, AOHeight);

        g_pTxtHelper->DrawFormattedTextLine(L"ZNear: %g, ZFar: %g", g_ProjectionMatrixDepthRange.ZNear, g_ProjectionMatrixDepthRange.ZFar);

#if ENABLE_RENDER_TIMES
#if ENABLE_INTERNAL_RENDER_TIMES
#if TEST_TWO_PASS_BLEND
        for (UINT PassIndex = 0; PassIndex < GFSDK::SSAO::RENDER_PASS_COUNT; ++PassIndex)
#else
        UINT PassIndex = 0;
#endif
        {
            g_pTxtHelper->DrawFormattedTextLine(L"GPU Times (ms): Total: %0.2f Z {%0.2f, %0.2f} AO {%0.2f, %0.2f, %0.2f} Blur {%0.2f, %0.2f}",
                GFSDK::SSAO::D3D11::Renderer::s_RenderTimes.GPUTimeMS[PassIndex][SSAO::REGIME_TIME_TOTAL],
                // Z
                GFSDK::SSAO::D3D11::Renderer::s_RenderTimes.GPUTimeMS[PassIndex][SSAO::REGIME_TIME_LINEAR_Z],
                GFSDK::SSAO::D3D11::Renderer::s_RenderTimes.GPUTimeMS[PassIndex][SSAO::REGIME_TIME_DEINTERLEAVE_Z],
                // AO
                GFSDK::SSAO::D3D11::Renderer::s_RenderTimes.GPUTimeMS[PassIndex][SSAO::REGIME_TIME_NORMAL],
                GFSDK::SSAO::D3D11::Renderer::s_RenderTimes.GPUTimeMS[PassIndex][SSAO::REGIME_TIME_COARSE_AO],
                GFSDK::SSAO::D3D11::Renderer::s_RenderTimes.GPUTimeMS[PassIndex][SSAO::REGIME_TIME_INTERLEAVE_AO],
                // Blur
                GFSDK::SSAO::D3D11::Renderer::s_RenderTimes.GPUTimeMS[PassIndex][SSAO::REGIME_TIME_BLURX],
                GFSDK::SSAO::D3D11::Renderer::s_RenderTimes.GPUTimeMS[PassIndex][SSAO::REGIME_TIME_BLURY]);
        }

        g_pTxtHelper->DrawFormattedTextLine(L"CPU Time (us): PreCreateRTs{%d} RenderAO{%d}", int(g_CPUTimePreCreateRTs), int(g_CPUTimeRenderAO));
#else
        g_pTxtHelper->DrawFormattedTextLine(L"HBAO+ GPU Time: %0.2f ms",
            GFSDK::SSAO::D3D11::Renderer::s_RenderTimes.GPUTimeMS[SSAO::REGIME_TIME_TOTAL]);
#endif
#endif

        g_pTxtHelper->DrawFormattedTextLine(L"Allocated Video Memory: %d MB\n", g_pAOContext->GetAllocatedVideoMemoryBytes() / (1024*1024));

#if ENABLE_NVAPI
        if (g_MonitorAvailableVidMem)
        {
            NvApiWrapper::MemInfo MemInfo = { 0 };
            g_NvApiWrapper.GetVideoMemoryInfo(&MemInfo);
            g_pTxtHelper->DrawFormattedTextLine(L"NVAPI: Current Available Video Memory: %u MB", MemInfo.CurrentAvailableDedicatedVideoMemoryInMB);
        }
        if (g_MonitorNvGraphicsClock)
        {
            NvApiWrapper::FrequencyInfo FreqInfo = { 0 };
            g_NvApiWrapper.GetNVGraphicsClockMhz(&FreqInfo);
            g_pTxtHelper->DrawFormattedTextLine(L"NVAPI Current NV GPU Graphics Clock: %d", FreqInfo.NvGraphicsClockInMhz);
        }
#endif

        g_pTxtHelper->End();
    }

    void OnRender(float fElapsedTime)
    {
#if ENABLE_HUD_BACKGROUND
        m_BackgroundQuad.OnRender(fElapsedTime);
#endif

        DrawText();
    }

private:
    CDXUTDialog m_BackgroundQuad;
};
TextRenderer g_TextRenderer;

//--------------------------------------------------------------------------------------
struct Cameras
{
    Cameras()
    {
        // Setup orbital camera
        D3DXVECTOR3 vecEye(0.0f, 2.0f, 0.0f);
        D3DXVECTOR3 vecAt (0.0f, 0.0f, 0.0f);
        Orbital.SetViewParams(&vecEye, &vecAt);
        Orbital.SetRadius(1.5f, 0.01f);
        Orbital.SetButtonMasks(MOUSE_LEFT_BUTTON, MOUSE_WHEEL, 0);

        // Setup first-person camera
        D3DXVECTOR3 sibenikVecEye(0.0960150138f, 0.0273544509f, -0.0185411610f);
        D3DXVECTOR3 sibenikVecAt (-0.623801112f, -0.649074197f, -0.174454257f);
        FirstPerson.SetViewParams(&sibenikVecEye, &sibenikVecAt);
        FirstPerson.SetEnablePositionMovement(1);
        FirstPerson.SetScalers(0.001f, 0.05f);
        FirstPerson.SetRotateButtons( 1, 1, 1 );
    }

    void SetProjParameters(SceneRTs *pRTs)
    {
        float Fovy = 2.f * atanf( tanf(FOVY * 0.5f) * (1.f + float(pRTs->BorderPixels) / (0.5f * float(pRTs->OutputHeight)) ) );
        float AspectRatio = float(pRTs->OutputWidth + pRTs->BorderPixels * 2) / float(pRTs->OutputHeight + pRTs->BorderPixels * 2);

        Orbital.SetProjParams (Fovy, AspectRatio, ZNEAR, ZFAR);
        Orbital.SetWindow     (pRTs->OutputWidth, pRTs->OutputHeight);
        FirstPerson.SetProjParams (Fovy, AspectRatio, ZNEAR, ZFAR);

#if TEST_REVERSED_DEPTH_TEST
        D3DXMatrixPerspectiveFovLH(&ProjectionMatrix, Fovy, AspectRatio, FLT_MAX, ZNEAR);
        ProjectionMatrix._33 = 0.f;
#endif
    }

    CFirstPersonCamera FirstPerson;
    CModelViewerCamera Orbital;
#if TEST_REVERSED_DEPTH_TEST
    D3DXMATRIX ProjectionMatrix;
#endif
};
Cameras g_Cameras;

//--------------------------------------------------------------------------------------
void GetAdapterDesc(ID3D11Device* pd3dDevice, DXGI_ADAPTER_DESC* pDesc)
{
    IDXGIDevice *pDXGIDevice = NULL;
    pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);

    IDXGIAdapter *pDXGIAdapter = NULL;
    pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&pDXGIAdapter);

    pDXGIAdapter->GetDesc(pDesc);

    SAFE_RELEASE(pDXGIAdapter);
    SAFE_RELEASE(pDXGIDevice);
}

//--------------------------------------------------------------------------------------
char* GetOutputFilename(ID3D11Device* pd3dDevice, char *FileExtension)
{
    static DXGI_ADAPTER_DESC AdapterDesc;
    if (AdapterDesc.DeviceId == 0)
    {
        GetAdapterDesc(pd3dDevice, &AdapterDesc);
    }

    static char FilePath[256];
    sprintf_s(FilePath, sizeof(FilePath), "..\\..\\Bin\\D3D11\\D3D11_%S_%dx%d.%s",
        AdapterDesc.Description,
        g_pSceneRTs->Width,
        g_pSceneRTs->Height,
        FileExtension
    );

    return FilePath;
}

//--------------------------------------------------------------------------------------
void InitAOParams(GFSDK_SSAO_Parameters &AOParams)
{
    AOParams = GFSDK_SSAO_Parameters();
    AOParams.Radius = 2.f;
    AOParams.Bias = 0.2f;
#if ENABLE_SMALL_SCALE_AO
    AOParams.SmallScaleAO = 1.f;
#endif
#if ENABLE_LARGE_SCALE_AO
    AOParams.LargeScaleAO = 1.f;
#endif
    AOParams.PowerExponent = 2.f;
    AOParams.Blur.Enable = g_BlurAO;
    AOParams.Blur.Sharpness = 16.f;
#if ENABLE_BLUR_RADIUS
    AOParams.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
#endif
#if ENABLE_BLUR_SHARPNESS_PROFILE
    AOParams.Blur.SharpnessProfile.Enable = TRUE;
    AOParams.Blur.SharpnessProfile.ForegroundSharpnessScale = 16.f;
    AOParams.Blur.SharpnessProfile.ForegroundViewDepth = 4.f;
    AOParams.Blur.SharpnessProfile.BackgroundViewDepth = 5.f;
#endif
#if ENABLE_VIEW_DEPTH_THRESHOLD
    AOParams.DepthThreshold.Enable = TRUE;
    AOParams.DepthThreshold.MaxViewDepth = MAX_DEPTH_THRESHOLD;
    AOParams.DepthThreshold.Sharpness = MAX_DEPTH_SHARPNESS;
#endif
#if ENABLE_BACKGROUND_AO
    AOParams.BackgroundAO.Enable = TRUE;
    AOParams.BackgroundAO.BackgroundViewDepth = MAX_BACKGROUND_DEPTH;
#endif
#if ENABLE_FOREGROUND_AO
    AOParams.ForegroundAO.Enable = TRUE;
    AOParams.ForegroundAO.ForegroundViewDepth = 0.f;
#endif
}

//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
void ReallocateSceneRTs(ID3D11Device* pd3dDevice)
{
    {
        SceneRTs::Desc desc;
        desc.OutputWidth = RENDER_AO_WIDTH;
        desc.OutputHeight = RENDER_AO_HEIGHT;
        desc.BorderPixels = g_BorderPixels;
        desc.SampleCount = g_MSAADesc[g_MSAACurrentSettings].SampleCount;

        SAFE_DELETE(g_pSceneRTs);
        g_pSceneRTs = new SceneRTs(pd3dDevice, desc);

        g_Cameras.SetProjParameters(g_pSceneRTs);
    }

#if TEST_PRE_CREATE_RTS
    {
        CPUTimer timer(&g_CPUTimePreCreateRTs);
        GFSDK_SSAO_Status status;
        status = g_pAOContext->PreCreateRTs(g_AOParams, g_BackBufferWidth, g_BackBufferHeight);
        assert(status == GFSDK_SSAO_OK);
    }
#endif
}

//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// DXUT will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
    assert(pDeviceSettings->ver == DXUT_D3D11_DEVICE);

    // Disable VSync
    pDeviceSettings->d3d11.SyncInterval = 0;

    // Do not allocate any depth buffer in DXUT
    pDeviceSettings->d3d11.AutoCreateDepthStencil = false;

#if defined(DEBUG)
    pDeviceSettings->d3d11.CreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // For the first device created if it is a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if (s_bFirstTime)
    {
        s_bFirstTime = false;
        if ((DXUT_D3D9_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF) ||
            (DXUT_D3D11_DEVICE == pDeviceSettings->ver &&
             pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE))
        {
            DXUTDisplaySwitchingToREFWarning(pDeviceSettings->ver);
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                      DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
    return true;
}

//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
void LoadScenes(ID3D11Device* pd3dDevice)
{
    int SceneId = 0;

#if ENABLE_MESHES
    // Load the sdkmesh data files
    for (int i = 0; i < SIZEOF_ARRAY(g_MeshDesc); ++i)
    {
        if (FAILED(g_SceneMeshes[i].OnCreateDevice(pd3dDevice, g_MeshDesc[i])))
        {
            MessageBox(NULL, L"Unable to create mesh", L"ERROR", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            PostQuitMessage(0);
        }
        g_Scenes[SceneId++].pMesh = &g_SceneMeshes[i];
    }
#endif
}

//--------------------------------------------------------------------------------------
#if TEST_TWO_PASS_BLEND
void CreateCustomBlendState(ID3D11Device* pd3dDevice)
{
    HRESULT hr;

    D3D11_BLEND_DESC BlendStateDesc;
    BlendStateDesc.AlphaToCoverageEnable = FALSE;
    BlendStateDesc.IndependentBlendEnable = TRUE;

    for (UINT i = 0; i < SIZEOF_ARRAY(BlendStateDesc.RenderTarget); ++i)
    {
        BlendStateDesc.RenderTarget[i].BlendEnable = FALSE;
        BlendStateDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }

    BlendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;

    BlendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    BlendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    BlendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MIN;

    BlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    BlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    BlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

    V( pd3dDevice->CreateBlendState(&BlendStateDesc, &g_pMinBlendState) );
}
#endif

//--------------------------------------------------------------------------------------
#if TEST_TWO_PASS_BLEND
void CreateCustomDepthStencilState(ID3D11Device* pd3dDevice)
{
    HRESULT hr;

    static D3D11_DEPTH_STENCIL_DESC DepthStencilStateDesc =
    {
        FALSE, //DepthEnable
        D3D11_DEPTH_WRITE_MASK_ZERO, //DepthWriteMask
        D3D11_COMPARISON_NEVER, //DepthFunc
        TRUE, //StencilEnable
        0xFF, //StencilReadMask
        0, //StencilWriteMask

        { D3D11_STENCIL_OP_KEEP, //StencilFailOp
        D3D11_STENCIL_OP_KEEP, //StencilDepthFailOp
        D3D11_STENCIL_OP_KEEP, //StencilPassOp
        D3D11_COMPARISON_EQUAL  //StencilFunc
        }, //FrontFace

        { D3D11_STENCIL_OP_KEEP, //StencilFailOp
        D3D11_STENCIL_OP_KEEP, //StencilDepthFailOp
        D3D11_STENCIL_OP_KEEP, //StencilPassOp
        D3D11_COMPARISON_EQUAL  //StencilFunc
        }  //BackFace
    };

    V(pd3dDevice->CreateDepthStencilState(&DepthStencilStateDesc, &g_pStencilEqualState));
}
#endif

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pD3D11Device, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
    HRESULT hr;

    DXUTTRACE(L"OnD3D11CreateDevice called\n");

    SetCursor(LoadCursor(0, IDC_ARROW));

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext(); // does not addref
    V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice(pD3D11Device, pd3dImmediateContext) );
    V_RETURN( g_SettingsDlg.OnD3D11CreateDevice(pD3D11Device) );
    g_pTxtHelper = new CDXUTTextHelper(pD3D11Device, pd3dImmediateContext, &g_DialogResourceManager, g_TextLineHeight);
    g_pSceneRenderer.OnCreateDevice(pD3D11Device, TEST_REVERSED_DEPTH_TEST);

    // Load meshes and bin files
    LoadScenes(pD3D11Device);

    GFSDK_SSAO_CustomHeap CustomHeap;
    CustomHeap.new_ = ::operator new;
    CustomHeap.delete_ = ::operator delete;

    GFSDK_SSAO_Status status;
    status = GFSDK_SSAO_CreateContext_D3D11(pD3D11Device, &g_pAOContext, &CustomHeap);
    assert(status == GFSDK_SSAO_OK);

#if TEST_TWO_PASS_BLEND
    CreateCustomBlendState(pD3D11Device);
    CreateCustomDepthStencilState(pD3D11Device);
#endif

    return S_OK;
}

//--------------------------------------------------------------------------------------
// SwapChain has changed and may have new attributes such as size.
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain *pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
    HRESULT hr;

    DXUTTRACE(L"OnD3D11ResizedSwapChain called\n");

    V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc) );
    V_RETURN( g_SettingsDlg.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc) );

    g_BackBufferWidth   = pBackBufferSurfaceDesc->Width;
    g_BackBufferHeight  = pBackBufferSurfaceDesc->Height;

    g_BackBufferViewport.TopLeftX = 0.f;
    g_BackBufferViewport.TopLeftY = 0.f;
    g_BackBufferViewport.MinDepth = 0.f;
    g_BackBufferViewport.MaxDepth = 1.f;
    g_BackBufferViewport.Width    = (FLOAT)g_BackBufferWidth;
    g_BackBufferViewport.Height   = (FLOAT)g_BackBufferHeight;

    UINT HudWidth = 256;
    float HudOpacity = 0.32f;
    g_HUD.SetLocation(g_BackBufferWidth - HudWidth, 0);
    g_HUD.SetSize    (HudWidth, g_BackBufferHeight);
    g_HUD.SetBackgroundColors(D3DCOLOR_COLORVALUE(0,0,0,HudOpacity));

    ReallocateSceneRTs(pd3dDevice);

    return hr;
}

//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
    SceneMesh *pSceneMesh = g_Scenes[g_CurrentSceneId].pMesh;
    g_UseOrbitalCamera = pSceneMesh && pSceneMesh->UseOrbitalCamera();

    if (g_UseOrbitalCamera)
    {
        g_Cameras.Orbital.FrameMove(fElapsedTime);
    }
    else
    {
        g_Cameras.FirstPerson.FrameMove(fElapsedTime);
    }
}

//--------------------------------------------------------------------------------------
void InitSceneViewInfo(SceneViewInfo &ViewInfo, UINT PassIndex)
{
    // PassIndex==0 is drawing the ground plane and the background mesh
    // PassIndex==1 is drawing the foreground mesh only
    ViewInfo.UseGBufferNormals = g_UseGBufferNormals;
    ViewInfo.AllowGroundPlane = PassIndex == 0;
    ViewInfo.StencilRef = PassIndex == 0 ? 1 : 2;

    if (g_UseOrbitalCamera)
    {
        const float TranslationZ = PassIndex == 1 ? 0.f : 0.4f;
        D3DXMATRIX TranslationMatrix;
        D3DXMatrixTranslation(&TranslationMatrix, 0.f, 0.f, TranslationZ);

        ViewInfo.WorldViewMatrix = TranslationMatrix * (*g_Cameras.Orbital.GetWorldMatrix()) * (*g_Cameras.Orbital.GetViewMatrix());
        ViewInfo.ProjectionMatrix = *g_Cameras.Orbital.GetProjMatrix();
    }
    else
    {
        D3DXMATRIX WorldMatrix;
        D3DXMatrixRotationX(&WorldMatrix, -D3DX_PI * 0.5f);
        ViewInfo.WorldViewMatrix = WorldMatrix * (*g_Cameras.FirstPerson.GetViewMatrix());
        ViewInfo.ProjectionMatrix = *g_Cameras.FirstPerson.GetProjMatrix();
    }

#if TEST_REVERSED_DEPTH_TEST
    ViewInfo.ProjectionMatrix = g_Cameras.ProjectionMatrix;
#endif
}

//--------------------------------------------------------------------------------------
void RenderAOFromMesh(ID3D11Device* pd3dDevice, 
                      ID3D11DeviceContext* pd3dImmediateContext,
                      ID3D11RenderTargetView* pBackBufferRTV,
                      SceneMesh *pMesh)
{
    //--------------------------------------------------------------------------------------
    // Set viewport
    //--------------------------------------------------------------------------------------
    D3D11_VIEWPORT Viewport;
    Viewport.TopLeftX = 0.f;
    Viewport.TopLeftY = 0.f;
    Viewport.MinDepth = 0.f;
    Viewport.MaxDepth = 1.f;
    Viewport.Width    = FLOAT(g_pSceneRTs->Width);
    Viewport.Height   = FLOAT(g_pSceneRTs->Height);

#if TEST_PARTIAL_VIEWPORT_DEPTH_RANGE
    Viewport.MinDepth = 0.2f;
    Viewport.MaxDepth = 0.8f;
#endif

    pd3dImmediateContext->RSSetViewports(1, &Viewport);

#if TEST_PARTIAL_VIEWPORT_DIMENSIONS
    if (g_TestSmallViewport)
    {
        Viewport.TopLeftX = 100.f;
        Viewport.TopLeftY = 100.f;
        Viewport.Width   -= 200.f;
        Viewport.Height  -= 200.f;
    }
#endif

    //--------------------------------------------------------------------------------------
    // Clear output AO render target
    //--------------------------------------------------------------------------------------
    float ClearColor[4] = { 1.f, 1.f, 1.f, 0.f };
    pd3dImmediateContext->ClearRenderTargetView(g_pSceneRTs->ColorRTV, ClearColor);

    //--------------------------------------------------------------------------------------
    // Clear scene depth-stencil buffer
    //--------------------------------------------------------------------------------------
#if TEST_REVERSED_DEPTH_TEST
    pd3dImmediateContext->ClearDepthStencilView(g_pSceneRTs->DepthStencilDSV, D3D11_CLEAR_DEPTH, 0.0, 0);
#else
    pd3dImmediateContext->ClearDepthStencilView(g_pSceneRTs->DepthStencilDSV, D3D11_CLEAR_DEPTH, 1.0, 0);
#endif

#if TEST_TWO_PASS_BLEND
    for (UINT PassIndex = 0; PassIndex < 2; ++PassIndex)
    {
    GFSDK::SSAO::RenderTimes::CurrentPassIndex = (GFSDK::SSAO::GpuTimeRenderPassIndex)PassIndex;
#else
    UINT PassIndex = 0;
#endif

    SceneViewInfo ViewInfo;
    InitSceneViewInfo(ViewInfo, PassIndex);

    //--------------------------------------------------------------------------------------
    // Render depth with the Scene3D class
    //--------------------------------------------------------------------------------------
    if (ViewInfo.UseGBufferNormals)
    {
        ID3D11RenderTargetView* pMRTs[] = { NULL, g_pSceneRTs->NormalRTV };
        pd3dImmediateContext->OMSetRenderTargets(SIZEOF_ARRAY(pMRTs), pMRTs, g_pSceneRTs->DepthStencilDSV);
    }
    else
    {
        pd3dImmediateContext->OMSetRenderTargets(0, NULL, g_pSceneRTs->DepthStencilDSV);
    }

    g_pSceneRenderer.RenderMesh(pd3dImmediateContext, &ViewInfo, pMesh);

    //--------------------------------------------------------------------------------------
    // Render the SSAO
    //--------------------------------------------------------------------------------------
    if (g_ShowAO)
    {
        GFSDK_SSAO_InputData_D3D11 Input;

#if TEST_PARTIAL_VIEWPORT_DEPTH_RANGE
        Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS_SUB_RANGE;
#else
        Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
#endif
        Input.DepthData.pFullResDepthTextureSRV = g_pSceneRTs->DepthStencilSRV;
        Input.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((CONST FLOAT*)ViewInfo.ProjectionMatrix);
        Input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
        Input.DepthData.MetersToViewSpaceUnits = pMesh->GetSceneScale();

#if TEST_PARTIAL_VIEWPORT_DIMENSIONS
        Input.DepthData.Viewport.Enable = TRUE;
        Input.DepthData.Viewport.TopLeftX = (GFSDK_SSAO_UINT)Viewport.TopLeftX;
        Input.DepthData.Viewport.TopLeftY = (GFSDK_SSAO_UINT)Viewport.TopLeftY;
        Input.DepthData.Viewport.Width = (GFSDK_SSAO_UINT)Viewport.Width;
        Input.DepthData.Viewport.Height = (GFSDK_SSAO_UINT)Viewport.Height;
        Input.DepthData.Viewport.MinDepth = Viewport.MinDepth;
        Input.DepthData.Viewport.MaxDepth = Viewport.MaxDepth;
#endif

        if (ViewInfo.UseGBufferNormals)
        {
            Input.NormalData.Enable = TRUE;
            Input.NormalData.pFullResNormalTextureSRV = g_pSceneRTs->NormalSRV;
            Input.NormalData.WorldToViewMatrix.Data = GFSDK_SSAO_Float4x4((CONST FLOAT*)ViewInfo.WorldViewMatrix);
            Input.NormalData.WorldToViewMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
            Input.NormalData.DecodeScale = 2.f;
            Input.NormalData.DecodeBias = -1.f;
        }

        GFSDK_SSAO_Output_D3D11 Output;
        Output.pRenderTargetView = g_pSceneRTs->ColorRTV;
        Output.Blend.Mode = GFSDK_SSAO_OVERWRITE_RGB;

#if TEST_TWO_PASS_BLEND
        if (PassIndex == 1)
        {
            Output.TwoPassBlend.Enable = g_EnableTwoPassBlend;
            Output.TwoPassBlend.pDepthStencilView = g_pSceneRTs->DepthStencilDSV;

            // No blending for stencil == 2 (foreground mesh, rendered last)
            Output.TwoPassBlend.FirstPass.Blend.Mode = GFSDK_SSAO_OVERWRITE_RGB;
            Output.TwoPassBlend.FirstPass.DepthStencil.Mode = GFSDK_SSAO_CUSTOM_DEPTH_STENCIL;
            Output.TwoPassBlend.FirstPass.DepthStencil.CustomState.pDepthStencilState = g_pStencilEqualState;
            Output.TwoPassBlend.FirstPass.DepthStencil.CustomState.StencilRef = 2;

            // MIN blending for stencil == 1
            Output.TwoPassBlend.SecondPass.Blend.Mode = GFSDK_SSAO_CUSTOM_BLEND;
            Output.TwoPassBlend.SecondPass.Blend.CustomState.pBlendState = g_pMinBlendState;
            Output.TwoPassBlend.SecondPass.DepthStencil.Mode = GFSDK_SSAO_CUSTOM_DEPTH_STENCIL;
            Output.TwoPassBlend.SecondPass.DepthStencil.CustomState.pDepthStencilState = g_pStencilEqualState;
            Output.TwoPassBlend.SecondPass.DepthStencil.CustomState.StencilRef = 1;
        }
#endif

        {
            CPUTimer timer(&g_CPUTimeRenderAO);
            GFSDK_SSAO_Status status;
            status = g_pAOContext->RenderAO(pd3dImmediateContext, Input, g_AOParams, Output, g_RenderMask);
            assert(status == GFSDK_SSAO_OK);
        }

        {
            GFSDK_SSAO_Status status;
            status = g_pAOContext->GetProjectionMatrixDepthRange(Input, g_ProjectionMatrixDepthRange);
            assert(status == GFSDK_SSAO_OK);
        }
    }

#if TEST_TWO_PASS_BLEND
    }
#endif

    //--------------------------------------------------------------------------------------
    // Copy/resolve colors
    //--------------------------------------------------------------------------------------

    D3D11_BOX SrcBox;
    SrcBox.left = g_BorderPixels;
    SrcBox.right = g_pSceneRTs->OutputWidth + g_BorderPixels;
    SrcBox.top = g_BorderPixels;
    SrcBox.bottom = g_pSceneRTs->OutputHeight + g_BorderPixels;
    SrcBox.front = 0;
    SrcBox.back = 1;

    if (g_pSceneRTs->SampleCount > 1)
    {
        pd3dImmediateContext->ResolveSubresource(g_pSceneRTs->ResolvedColorTexture, 0, g_pSceneRTs->ColorTexture, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
        pd3dImmediateContext->CopySubresourceRegion(g_pSceneRTs->OutputTexture, 0, 0, 0, 0, g_pSceneRTs->ResolvedColorTexture, 0, &SrcBox);
    }
    else
    {
        pd3dImmediateContext->CopySubresourceRegion(g_pSceneRTs->OutputTexture, 0, 0, 0, 0, g_pSceneRTs->ColorTexture, 0, &SrcBox);
    }

    //--------------------------------------------------------------------------------------
    // Dump screenshot on first frame
    //--------------------------------------------------------------------------------------

    static bool DumpedShot = false;
    if (!DumpedShot)
    {
        DumpedShot = true;

        HRESULT hr;
        hr = D3DX11SaveTextureToFileA(pd3dImmediateContext, g_pSceneRTs->OutputTexture, D3DX11_IFF_BMP, GetOutputFilename(pd3dDevice,"bmp"));
        assert(hr == S_OK);
    }

    //--------------------------------------------------------------------------------------
    // Copy/stretch the output colors to the backbuffer
    //--------------------------------------------------------------------------------------

    pd3dImmediateContext->OMSetRenderTargets(1, &pBackBufferRTV, NULL);
    pd3dImmediateContext->RSSetViewports(1, &g_BackBufferViewport);
    g_pSceneRenderer.CopyColors(pd3dImmediateContext, g_pSceneRTs->OutputSRV);
}

//--------------------------------------------------------------------------------------
// Callback function that renders the frame.  This function sets up the rendering 
// matrices and renders the scene and UI.
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, 
                                 double fTime, float fElapsedTime, void* pUserContext)
{
    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if (g_SettingsDlg.IsActive())
    {
        g_SettingsDlg.OnRender(fElapsedTime);
        return;
    }

    // Reallocate the render targets and depth buffer if the MSAA mode has been changed in the GUI.
    int SelectedId = g_HUD.GetComboBox(IDC_CHANGEMSAA)->GetSelectedIndex();
    if (g_MSAACurrentSettings != SelectedId)
    {
        g_MSAACurrentSettings = SelectedId;
        ReallocateSceneRTs(pd3dDevice);
    }

    if (g_BorderPixels != g_pSceneRTs->BorderPixels)
    {
        ReallocateSceneRTs(pd3dDevice);
    }

    SceneMesh *pMesh = g_Scenes[g_CurrentSceneId].pMesh;

    // These have no effect if the mesh is not rendered
    g_HUD.GetComboBox(IDC_CHANGEMSAA)->SetVisible(pMesh != NULL);

#if ENABLE_BLUR_RADIUS
    g_HUD.GetStatic(IDC_BLUR_SHARPNESS_STATIC)->SetVisible(g_BlurAO);
    g_HUD.GetSlider(IDC_BLUR_SHARPNESS_SLIDER)->SetVisible(g_BlurAO);
#endif

    g_AOParams.Blur.Enable = g_BlurAO;
    g_RenderMask = g_DebugNormals ? GFSDK_SSAO_RENDER_DEBUG_NORMAL : GFSDK_SSAO_RENDER_AO;

    ID3D11RenderTargetView* pBackBufferRTV = DXUTGetD3D11RenderTargetView(); // does not addref

    if (pMesh)
    {
        RenderAOFromMesh(pd3dDevice, 
                         pd3dImmediateContext,
                         pBackBufferRTV,
                         pMesh);
    }

    pd3dImmediateContext->RSSetViewports(1, &g_BackBufferViewport);

    if (!g_ShowColors && !g_ShowAO)
    {
        // If both "Show Colors" and "Show AO" are disabled in the GUI
        float ClearColor[4] = { 0.0f, 0.0f, 0.0f };
        pd3dImmediateContext->ClearRenderTargetView(pBackBufferRTV, ClearColor);
    }

    //--------------------------------------------------------------------------------------
    // Render the GUI
    //--------------------------------------------------------------------------------------
    if (g_DrawUI && !g_GPUProfiler.IsProfiling())
    {
        g_HUD.OnRender(fElapsedTime);
        g_TextRenderer.OnRender(fElapsedTime);
    }

    g_GPUProfiler.Tick(GFSDK::SSAO::D3D11::Renderer::s_RenderTimes, GetOutputFilename(pd3dDevice,"csv"));
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
    DXUTTRACE(L"OnD3D11ReleasingSwapChain called\n");

    g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles.
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
    DXUTTRACE(L"OnD3D11DestroyDevice called\n");

    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_SettingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_DELETE(g_pTxtHelper);

    g_pSceneRenderer.OnDestroyDevice();

#if ENABLE_MESHES
    for (int i = 0; i < SIZEOF_ARRAY(g_MeshDesc); i++)
    {
        g_SceneMeshes[i].OnDestroyDevice();
    }
#endif

    SAFE_DELETE(g_pSceneRTs);

#if TEST_TWO_PASS_BLEND
    SAFE_RELEASE(g_pMinBlendState);
    SAFE_RELEASE(g_pStencilEqualState);
#endif

    SAFE_RELEASE(g_pAOContext);
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
                          bool* pbNoFurtherProcessing, void* pUserContext)
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
    if (*pbNoFurtherProcessing)
    {
        return 0;
    }

    if (g_SettingsDlg.IsActive())
    {
        g_SettingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc(hWnd, uMsg, wParam, lParam);
    if (*pbNoFurtherProcessing)
    {
        return 0;
    }

    // Pass all windows messages to camera so it can respond to user input
    if (g_UseOrbitalCamera)
    {
        g_Cameras.Orbital.HandleMessages(hWnd, uMsg, wParam, lParam);
    }
    else
    {
        g_Cameras.FirstPerson.HandleMessages(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
    switch (nControlID)
    {
        case IDC_TOGGLEFULLSCREEN:   DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:          DXUTToggleREF();        break;
           
        case IDC_CHANGEDEVICE:
        {
            g_SettingsDlg.SetActive(!g_SettingsDlg.IsActive()); 
            break;
        }
        case IDC_CHANGESCENE:
        {
            CDXUTComboBox* pComboBox = (CDXUTComboBox*) pControl;
            g_CurrentSceneId = pComboBox->GetSelectedIndex();
            break;
        }
#if ENABLE_SHOW_COLORS
        case IDC_SHOW_COLORS:
        {
            g_ShowColors = g_HUD.GetCheckBox(IDC_SHOW_COLORS)->GetChecked();
            break;
        }
        case IDC_SHOW_AO:
        {
            g_ShowAO = g_HUD.GetCheckBox(IDC_SHOW_AO)->GetChecked();
            break;
        }
#endif
#if ENABLE_GBUFFER_NORMALS
        case IDC_USE_GBUFFER_NORMALS:
        {
            g_UseGBufferNormals = g_HUD.GetCheckBox(IDC_USE_GBUFFER_NORMALS)->GetChecked();
            break;
        }
#endif
#if ENABLE_DEBUG_NORMALS
        case IDC_DEBUG_NORMALS:
        {
            g_DebugNormals = g_HUD.GetCheckBox(IDC_DEBUG_NORMALS)->GetChecked();
            break;
        }
#endif
        case IDC_RADIUS_SLIDER:
        {
            g_AOParams.Radius = (float) g_HUD.GetSlider(IDC_RADIUS_SLIDER)->GetValue() * MAX_RADIUS_MULT / 100.0f;

            WCHAR sz[100];
            StringCchPrintf(sz, 100, UI_RADIUS_MULT L"%0.2f", g_AOParams.Radius); 
            g_HUD.GetStatic(IDC_RADIUS_STATIC)->SetText(sz);

            break;
        }
#if ENABLE_BACKGROUND_AO
        case IDC_BACKGROUND_DEPTH_SLIDER:
        {
            g_AOParams.BackgroundAO.BackgroundViewDepth = (float) g_HUD.GetSlider(IDC_BACKGROUND_DEPTH_SLIDER)->GetValue() * MAX_BACKGROUND_DEPTH / 10000.0f;

            WCHAR sz[100];
            StringCchPrintf(sz, 100, UI_BACKGROUND_DEPTH L"%0.2f", g_AOParams.BackgroundAO.BackgroundViewDepth);
            g_HUD.GetStatic(IDC_BACKGROUND_DEPTH_STATIC)->SetText(sz);

            break;
        }
#endif
#if ENABLE_FOREGROUND_AO
        case IDC_FOREGROUND_DEPTH_SLIDER:
        {
            g_AOParams.ForegroundAO.ForegroundViewDepth = (float)g_HUD.GetSlider(IDC_FOREGROUND_DEPTH_SLIDER)->GetValue() * MAX_BACKGROUND_DEPTH / 10000.0f;

            WCHAR sz[100];
            StringCchPrintf(sz, 100, UI_FOREGROUND_DEPTH L"%0.2f", g_AOParams.ForegroundAO.ForegroundViewDepth);
            g_HUD.GetStatic(IDC_FOREGROUND_DEPTH_STATIC)->SetText(sz);

            break;
        }
#endif
#if ENABLE_VIEW_DEPTH_THRESHOLD
        case IDC_DEPTH_THRESHOLD_SLIDER:
        {
            g_AOParams.DepthThreshold.MaxViewDepth = (float) g_HUD.GetSlider(IDC_DEPTH_THRESHOLD_SLIDER)->GetValue() * MAX_DEPTH_THRESHOLD / 10000.0f;

            WCHAR sz[100];
            StringCchPrintf(sz, 100, UI_DEPTH_THRESHOLD L"%0.2f", g_AOParams.DepthThreshold.MaxViewDepth);
            g_HUD.GetStatic(IDC_DEPTH_THRESHOLD_STATIC)->SetText(sz);

            break;
        }
        case IDC_DEPTH_SHARPNESS_SLIDER:
        {
            g_AOParams.DepthThreshold.Sharpness = (float) g_HUD.GetSlider(IDC_DEPTH_SHARPNESS_SLIDER)->GetValue() * MAX_DEPTH_SHARPNESS / 10000.0f;

            WCHAR sz[100];
            StringCchPrintf(sz, 100, UI_DEPTH_SHARPNESS L"%0.1f", g_AOParams.DepthThreshold.Sharpness);
            g_HUD.GetStatic(IDC_DEPTH_SHARPNESS_STATIC)->SetText(sz);

            break;
        }
#endif
        case IDC_BIAS_SLIDER:
        {
            g_AOParams.Bias = (float) g_HUD.GetSlider(IDC_BIAS_SLIDER)->GetValue() / 1000.f;

            WCHAR sz[100];
            StringCchPrintf(sz, 100, UI_AO_BIAS L"%g", g_AOParams.Bias); 
            g_HUD.GetStatic(IDC_BIAS_STATIC)->SetText(sz);

            break;
        }
#if ENABLE_SMALL_SCALE_AO
        case IDC_SMALL_SCALE_AO_SLIDER:
        {
            g_AOParams.SmallScaleAO = (float)g_HUD.GetSlider(IDC_SMALL_SCALE_AO_SLIDER)->GetValue() / 100.0f;

            WCHAR sz[100];
            StringCchPrintf(sz, 100, UI_SMALL_SCALE_AO L"%0.2f", g_AOParams.SmallScaleAO);
            g_HUD.GetStatic(IDC_SMALL_SCALE_AO_STATIC)->SetText(sz);

            break;
        }
#endif
#if ENABLE_LARGE_SCALE_AO
        case IDC_LARGE_SCALE_AO_SLIDER:
        {
            g_AOParams.LargeScaleAO = (float)g_HUD.GetSlider(IDC_LARGE_SCALE_AO_SLIDER)->GetValue() / 100.0f;

            WCHAR sz[100];
            StringCchPrintf(sz, 100, UI_LARGE_SCALE_AO L"%0.2f", g_AOParams.LargeScaleAO);
            g_HUD.GetStatic(IDC_LARGE_SCALE_AO_STATIC)->SetText(sz);

            break;
        }
#endif
        case IDC_EXPONENT_SLIDER: 
        {
            g_AOParams.PowerExponent = (float)g_HUD.GetSlider(IDC_EXPONENT_SLIDER)->GetValue() / 100.0f;

            WCHAR sz[100];
            StringCchPrintf(sz, 100, UI_POW_EXPONENT L"%0.2f", g_AOParams.PowerExponent);
            g_HUD.GetStatic(IDC_EXPONENT_STATIC)->SetText(sz);

            break;
        }
#if ENABLE_BLUR_RADIUS
        case IDC_BLUR_DISABLED:
        {
            g_BlurAO = false;
            break;
        }
        case IDC_BLUR_RADIUS_2:
        {
            g_BlurAO = true;
            g_AOParams.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_2;
            break;
        }
        case IDC_BLUR_RADIUS_4:
        {
            g_BlurAO = true;
            g_AOParams.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
            break;
        }
#endif
#if ENABLE_BORDER_PIXELS
        case IDC_BORDER_PIXELS_0:
        {
            g_BorderPixels = 0;
            break;
        }
        case IDC_BORDER_PIXELS_64:
        {
            g_BorderPixels = 64;
            break;
        }
        case IDC_BORDER_PIXELS_128:
        {
            g_BorderPixels = 128;
            break;
        }
#endif
#if ENABLE_BLUR_SHARPNESS
        case IDC_BLUR_SHARPNESS_SLIDER: 
        {
            g_AOParams.Blur.Sharpness = (float)g_HUD.GetSlider(IDC_BLUR_SHARPNESS_SLIDER)->GetValue() * MAX_BLUR_SHARPNESS / 10000.0f;

            WCHAR sz[100];
            StringCchPrintf(sz, 100, UI_BLUR_SHARPNESS L"%0.2f", g_AOParams.Blur.Sharpness);
            g_HUD.GetStatic(IDC_BLUR_SHARPNESS_STATIC)->SetText(sz);

            break;
        }
#endif
#if ENABLE_FP16_VIEW_DEPTHS
        case IDC_DEPTH_STORAGE_TYPE_FP16:
        {
            g_AOParams.DepthStorage = GFSDK_SSAO_FP16_VIEW_DEPTHS;
            break;
        }
        case IDC_DEPTH_STORAGE_TYPE_FP32:
        {
            g_AOParams.DepthStorage = GFSDK_SSAO_FP32_VIEW_DEPTHS;
            break;
        }
#endif
#if ENABLE_DEPTH_CLAMP_MODES
        case IDC_DEPTH_CLAMP_TO_EDGE:
        {
            g_AOParams.DepthClampMode = GFSDK_SSAO_CLAMP_TO_EDGE;
            break;
        }
        case IDC_DEPTH_CLAMP_TO_BORDER:
        {
            g_AOParams.DepthClampMode = GFSDK_SSAO_CLAMP_TO_BORDER;
            break;
        }
#endif
#if ENABLE_DEBUG_MODES
        case IDC_SHADER_TYPE_HBAO_PLUS:
        {
            GFSDK::SSAO::D3D11::Renderer::s_AOShaderType = GFSDK::SSAO::D3D11::Renderer::NVSDK_HBAO_PLUS_PS;
            break;
        }
        case IDC_SHADER_TYPE_DEBUG_HBAO_PLUS:
        {
            GFSDK::SSAO::D3D11::Renderer::s_AOShaderType = GFSDK::SSAO::D3D11::Renderer::NVSDK_DEBUG_HBAO_PLUS_PS;
            break;
        }
#endif
    }
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
    if (bKeyDown)
    {
        switch (nChar)
        {
#if TEST_PARTIAL_VIEWPORT_DIMENSIONS
            case 'V':
                g_TestSmallViewport = !g_TestSmallViewport;
                break;
#endif
#if ENABLE_NVAPI
            case 'M':
                g_MonitorAvailableVidMem = !g_MonitorAvailableVidMem;
                break;
            case 'C':
                g_MonitorNvGraphicsClock = !g_MonitorNvGraphicsClock;
                break;
#endif
            case 'U':
                g_DrawUI = !g_DrawUI;
                break;
            case 'P':
                g_GPUProfiler.StartProfiling();
                break;
#if TEST_TWO_PASS_BLEND
            case 'T':
                g_EnableTwoPassBlend = !g_EnableTwoPassBlend;
                break;
#endif
        }
    }
}

//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, 
                       bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta, 
                       int xPos, int yPos, void* pUserContext)
{
}

//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved(void* pUserContext)
{
    return true;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void InitGUI()
{
    // Initialize dialogs
    g_SettingsDlg.Init(&g_DialogResourceManager);
    g_HUD.Init(&g_DialogResourceManager);
    g_HUD.SetCallback(OnGUIEvent);
    g_TextRenderer.Init(&g_DialogResourceManager);
    
    int iY = 10; 
    g_HUD.AddButton  (IDC_TOGGLEFULLSCREEN,     L"Toggle full screen" ,   35, iY, 160, 22);
    g_HUD.AddButton  (IDC_TOGGLEREF,            L"Toggle REF (F3)"    ,   35, iY += 24, 160, 22, VK_F3);
    g_HUD.AddButton  (IDC_CHANGEDEVICE,         L"Change device (F2)" ,   35, iY += 24, 160, 22, VK_F2);
    iY += 20;

#if ENABLE_SHOW_COLORS
    g_HUD.AddCheckBox(IDC_SHOW_COLORS,          L"Show Colors",             35, iY += 26, 125, 22, g_ShowColors);
    g_HUD.AddCheckBox(IDC_SHOW_AO,              L"Show HBAO+",              35, iY += 26, 125, 22, g_ShowAO);
#endif
#if ENABLE_DEBUG_NORMALS
    g_HUD.AddCheckBox(IDC_DEBUG_NORMALS,        L"Debug Normals",           35, iY += 26, 125, 22, g_DebugNormals);
#endif
#if ENABLE_GBUFFER_NORMALS
    g_HUD.AddCheckBox(IDC_USE_GBUFFER_NORMALS,  L"GBuffer Normals",         35, iY += 26, 125, 22, g_UseGBufferNormals);
#endif
    iY += 20;

    CDXUTComboBox *pComboBox;
    g_HUD.AddComboBox(IDC_CHANGESCENE,    35, iY += 24, 160, 22, 'S', false, &pComboBox);
#if ENABLE_MESHES
    for (int i = 0; i < SIZEOF_ARRAY(g_MeshDesc); i++)
    {
        pComboBox->AddItem(g_MeshDesc[i].Name, NULL);
    }
#endif

    g_HUD.AddComboBox(IDC_CHANGEMSAA,    35, iY += 24, 160, 22, 'N', false, &pComboBox);
    for (int i = 0; g_MSAADesc[i].SampleCount != 0; i++)
    {
        pComboBox->AddItem(g_MSAADesc[i].Name, (void*)i);
    }
    pComboBox->SetSelectedByIndex(g_MSAACurrentSettings);
    iY += 20;

    WCHAR sz[100];
    int dy = 20;
    StringCchPrintf(sz, 100, UI_RADIUS_MULT L"%0.2f", g_AOParams.Radius); 
    g_HUD.AddStatic(IDC_RADIUS_STATIC, sz, 35, iY += dy, 125, 22);
    g_HUD.AddSlider(IDC_RADIUS_SLIDER, 50, iY += dy, 100, 22, 0, 100, int(g_AOParams.Radius / MAX_RADIUS_MULT * 100));

#if ENABLE_VIEW_DEPTH_THRESHOLD
    StringCchPrintf(sz, 100, UI_DEPTH_THRESHOLD L"%0.2f", g_AOParams.DepthThreshold.MaxViewDepth); 
    g_HUD.AddStatic(IDC_DEPTH_THRESHOLD_STATIC, sz, 35, iY += dy, 125, 22);
    g_HUD.AddSlider(IDC_DEPTH_THRESHOLD_SLIDER, 50, iY += dy, 100, 22, 0, 10000, int(g_AOParams.DepthThreshold.MaxViewDepth / MAX_DEPTH_THRESHOLD * 10000));

    StringCchPrintf(sz, 100, UI_DEPTH_SHARPNESS L"%0.1f", g_AOParams.DepthThreshold.Sharpness); 
    g_HUD.AddStatic(IDC_DEPTH_SHARPNESS_STATIC, sz, 35, iY += dy, 125, 22);
    g_HUD.AddSlider(IDC_DEPTH_SHARPNESS_SLIDER, 50, iY += dy, 100, 22, 0, 10000, int(g_AOParams.DepthThreshold.Sharpness / MAX_DEPTH_SHARPNESS * 10000));
#endif

#if ENABLE_BACKGROUND_AO
    StringCchPrintf(sz, 100, UI_BACKGROUND_DEPTH L"%0.2f", g_AOParams.BackgroundAO.BackgroundViewDepth);
    g_HUD.AddStatic(IDC_BACKGROUND_DEPTH_STATIC, sz, 35, iY += dy, 125, 22);
    g_HUD.AddSlider(IDC_BACKGROUND_DEPTH_SLIDER, 50, iY += dy, 100, 22, 1000, 10000, int(g_AOParams.BackgroundAO.BackgroundViewDepth / MAX_BACKGROUND_DEPTH * 10000));
#endif

#if ENABLE_FOREGROUND_AO
    StringCchPrintf(sz, 100, UI_FOREGROUND_DEPTH L"%0.2f", g_AOParams.ForegroundAO.ForegroundViewDepth);
    g_HUD.AddStatic(IDC_FOREGROUND_DEPTH_STATIC, sz, 35, iY += dy, 125, 22);
    g_HUD.AddSlider(IDC_FOREGROUND_DEPTH_SLIDER, 50, iY += dy, 100, 22, 1000, 10000, int(g_AOParams.ForegroundAO.ForegroundViewDepth / MAX_BACKGROUND_DEPTH * 10000));
#endif

    StringCchPrintf(sz, 100, UI_AO_BIAS L"%g", g_AOParams.Bias);
    g_HUD.AddStatic(IDC_BIAS_STATIC, sz, 35, iY += dy, 125, 22);
    g_HUD.AddSlider(IDC_BIAS_SLIDER, 50, iY += dy, 100, 22, 0, 500, int(g_AOParams.Bias * 1000));

#if ENABLE_SMALL_SCALE_AO
    StringCchPrintf(sz, 100, UI_SMALL_SCALE_AO L"%0.2f", g_AOParams.SmallScaleAO);
    g_HUD.AddStatic(IDC_SMALL_SCALE_AO_STATIC, sz, 35, iY += dy, 125, 22);
    g_HUD.AddSlider(IDC_SMALL_SCALE_AO_SLIDER, 50, iY += dy, 100, 22, 0, 200, (int)(100.0f*g_AOParams.SmallScaleAO));
#endif

#if ENABLE_LARGE_SCALE_AO
    StringCchPrintf(sz, 100, UI_LARGE_SCALE_AO L"%0.2f", g_AOParams.LargeScaleAO);
    g_HUD.AddStatic(IDC_LARGE_SCALE_AO_STATIC, sz, 35, iY += dy, 125, 22);
    g_HUD.AddSlider(IDC_LARGE_SCALE_AO_SLIDER, 50, iY += dy, 100, 22, 0, 200, (int)(100.0f*g_AOParams.LargeScaleAO));
#endif

    StringCchPrintf(sz, 100, UI_POW_EXPONENT L"%0.2f", g_AOParams.PowerExponent); 
    g_HUD.AddStatic(IDC_EXPONENT_STATIC, sz, 35, iY += dy, 125, 22);
    g_HUD.AddSlider(IDC_EXPONENT_SLIDER, 50, iY += dy, 100, 22, 0, 400, (int)(100.0f*g_AOParams.PowerExponent));

#if ENABLE_BLUR_SHARPNESS
    StringCchPrintf(sz, 100, UI_BLUR_SHARPNESS L"%0.2f", g_AOParams.Blur.Sharpness); 
    g_HUD.AddStatic(IDC_BLUR_SHARPNESS_STATIC, sz, 35, iY += dy, 125, 22);
    g_HUD.AddSlider(IDC_BLUR_SHARPNESS_SLIDER, 50, iY += dy, 100, 22, 0, 10000, int(g_AOParams.Blur.Sharpness / MAX_DEPTH_SHARPNESS * 10000));
#endif

    UINT ButtonGroup = 0;

#if ENABLE_DEBUG_MODES
    ButtonGroup++;
    iY += 20;
    g_HUD.AddRadioButton( IDC_SHADER_TYPE_HBAO_PLUS,         ButtonGroup, L"HBAO+",             35, iY += 24, 125, 22, (GFSDK::SSAO::D3D11::Renderer::s_AOShaderType == GFSDK::SSAO::D3D11::Renderer::NVSDK_HBAO_PLUS_PS) );
    g_HUD.AddRadioButton( IDC_SHADER_TYPE_DEBUG_HBAO_PLUS,   ButtonGroup, L"DEBUG_HBAO+",       35, iY += 24, 125, 22, (GFSDK::SSAO::D3D11::Renderer::s_AOShaderType == GFSDK::SSAO::D3D11::Renderer::NVSDK_DEBUG_HBAO_PLUS_PS) );
#endif

#if ENABLE_BLUR_RADIUS
    ButtonGroup++;
    iY += 20;
    g_HUD.AddRadioButton( IDC_BLUR_DISABLED,   ButtonGroup, L"BLUR_DISABLED",                   35, iY += 24, 125, 22,  !g_BlurAO );
    g_HUD.AddRadioButton( IDC_BLUR_RADIUS_2,   ButtonGroup, L"BLUR_RADIUS_2",                   35, iY += 24, 125, 22,  g_BlurAO && (g_AOParams.Blur.Radius == GFSDK_SSAO_BLUR_RADIUS_2) );
    g_HUD.AddRadioButton( IDC_BLUR_RADIUS_4,   ButtonGroup, L"BLUR_RADIUS_4",                   35, iY += 24, 125, 22,  g_BlurAO && (g_AOParams.Blur.Radius == GFSDK_SSAO_BLUR_RADIUS_4) );
#endif

#if ENABLE_FP16_VIEW_DEPTHS
    ButtonGroup++;
    iY += 20;
    g_HUD.AddRadioButton( IDC_DEPTH_STORAGE_TYPE_FP16,   ButtonGroup, L"FP16_VIEW_DEPTHS",  35, iY += 24, 125, 22,  (g_AOParams.DepthStorage == GFSDK_SSAO_FP16_VIEW_DEPTHS) );
    g_HUD.AddRadioButton( IDC_DEPTH_STORAGE_TYPE_FP32,   ButtonGroup, L"FP32_VIEW_DEPTHS",  35, iY += 24, 125, 22,  (g_AOParams.DepthStorage == GFSDK_SSAO_FP32_VIEW_DEPTHS) );
#endif

#if ENABLE_DEPTH_CLAMP_MODES
    ButtonGroup++;
    iY += 20;
    g_HUD.AddRadioButton( IDC_DEPTH_CLAMP_TO_EDGE,      ButtonGroup, L"CLAMP_TO_EDGE",      35, iY += 24, 125, 22,  (g_AOParams.DepthClampMode == GFSDK_SSAO_CLAMP_TO_EDGE) );
    g_HUD.AddRadioButton( IDC_DEPTH_CLAMP_TO_BORDER,    ButtonGroup, L"CLAMP_TO_BORDER",    35, iY += 24, 125, 22,  (g_AOParams.DepthClampMode == GFSDK_SSAO_CLAMP_TO_BORDER) );
#endif

#if ENABLE_BORDER_PIXELS
    ButtonGroup++;
    iY += 20;
    g_HUD.AddRadioButton( IDC_BORDER_PIXELS_0,      ButtonGroup, L"BORDER_PIXELS_0",            35, iY += 24, 125, 22, (g_BorderPixels == 0) );
    g_HUD.AddRadioButton( IDC_BORDER_PIXELS_64,     ButtonGroup, L"BORDER_PIXELS_64",           35, iY += 24, 125, 22, (g_BorderPixels == 64) );
    g_HUD.AddRadioButton( IDC_BORDER_PIXELS_128,    ButtonGroup, L"BORDER_PIXELS_128",          35, iY += 24, 125, 22, (g_BorderPixels == 128) );
#endif
}

//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    HRESULT hr;
    V_RETURN(DXUTSetMediaSearchPath(L"..\\..\\assets"));

    // Enable run-time memory check for debug builds.
#if defined(DEBUG) && defined (DEBUG_CRT)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // Set general DXUT callbacks
    DXUTSetCallbackFrameMove(OnFrameMove);
    DXUTSetCallbackKeyboard(OnKeyboard);
    DXUTSetCallbackMouse(OnMouse);
    DXUTSetCallbackMsgProc(MsgProc);
    DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
    DXUTSetCallbackDeviceRemoved(OnDeviceRemoved);

    // Set the D3D11 DXUT callbacks
    DXUTSetCallbackD3D11DeviceAcceptable  (IsD3D11DeviceAcceptable);
    DXUTSetCallbackD3D11DeviceCreated     (OnD3D11CreateDevice);
    DXUTSetCallbackD3D11SwapChainResized  (OnD3D11ResizedSwapChain);
    DXUTSetCallbackD3D11FrameRender       (OnD3D11FrameRender);
    DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
    DXUTSetCallbackD3D11DeviceDestroyed   (OnD3D11DestroyDevice);

    // Perform any application-level initialization here
    InitAOParams(g_AOParams);
    InitGUI();

    UINT Width = 1280;
    UINT Height = 720;

    DXUTInit(true, true, NULL); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings(true, true); // Show the cursor and clip it when in full screen
    DXUTSetIsInGammaCorrectMode(false); // Do not use a SRGB back buffer for this sample
    DXUTCreateWindow(L"NVIDIA HBAO+");
    DXUTCreateDevice(D3D_FEATURE_LEVEL_11_0, true, Width, Height);
    //DXUTToggleFullScreen();

    DXUTMainLoop(); // Enter into the DXUT render loop

    // Perform any application-level cleanup here

    return DXUTGetExitCode();
}
