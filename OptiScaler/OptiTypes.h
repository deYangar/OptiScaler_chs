#pragma once
#include <string>

/**
 * @brief Common strings and identifiers used internally by OptiScaler
 */
namespace OptiKeys
{
using CString = const char[];

// Application name provided to upscalers
inline constexpr CString ProjectID = "OptiScaler";

// ID code used for the Vulkan input provider
inline constexpr CString VkProvider = "OptiVk";
// ID code used for the DX11 input provider
inline constexpr CString Dx11Provider = "OptiDx11";
// ID code used for the DX12 input provider
inline constexpr CString Dx12Provider = "OptiDx12";

inline constexpr CString FSR_UpscaleWidth = "FSR.upscaleSize.width";
inline constexpr CString FSR_UpscaleHeight = "FSR.upscaleSize.height";

inline constexpr CString FSR_NearPlane = "FSR.cameraNear";
inline constexpr CString FSR_FarPlane = "FSR.cameraFar";
inline constexpr CString FSR_CameraFovVertical = "FSR.cameraFovAngleVertical";
inline constexpr CString FSR_FrameTimeDelta = "FSR.frameTimeDelta";
inline constexpr CString FSR_ViewSpaceToMetersFactor = "FSR.viewSpaceToMetersFactor";
inline constexpr CString FSR_TransparencyAndComp = "FSR.transparencyAndComposition";
inline constexpr CString FSR_Reactive = "FSR.reactive";

} // namespace OptiKeys

typedef enum API
{
    NotSelected = 0,
    DX11,
    DX12,
    Vulkan,
} API;

enum class Upscaler
{

    XeSS, // "xess", used for the native XeSS upscaler backend

    XeSS_on12, // "xess_12", DirectX 12 upscaler with an appropriate compatibility layer

    FSR21, // "fsr21", used for the native FSR 2.1.x upscaler backend

    FSR21_on12, // "fsr21_12", DirectX 12 upscaler with an appropriate compatibility layer

    FSR22, // "fsr22", used for the native FSR 2.2.x upscaler backend

    FSR22_on12, // "fsr22_12", DirectX 12 upscaler with an appropriate compatibility layer

    FSR31, // "fsr31", native DX11 version of FSR 3.1

    FFX, // "ffx", used for the native FSR 2.3; 3.1; 4.x

    FFX_on12, // "ffx_12", DirectX 12 upscaler with an appropriate compatibility layer

    DLSS, // "dlss", used for the DLSS upscaler backend

    DLSSD, // "dlssd", used for the DLSS-D/Ray Reconstruction upscaler+denoiser backend
    Reset
};

enum class ApiUpscalerInput
{
    DLSS_DX11,
    DLSS_DX12,
    DLSS_VK,
    XeSS_DX11,
    XeSS_DX12,
    XeSS_VK,
    FFX_DX12,
    FFX_VK,
    FSR20_DX12,
    FSR2X_DX11,
    FSR2X_DX12,
    FSR2X_VK,
    FSR2_TinyTina,
    FSR3_DX12,
};

enum class SharpenShader
{
    RCAS,
    DepthAware,
    LocalContrastDepthAware
};

enum class FSR4Support : uint8_t
{
    None = 0,
    FP8 = 1,
    INT8 = 2,
    Count
};

std::string ApiUpscalerInputName(ApiUpscalerInput upscaler);

std::string UpscalerDisplayName(Upscaler upscaler, API api = API::NotSelected);
std::string UpscalerShortName(Upscaler upscaler);
bool IsFsr(Upscaler upscaler);

// Converts enum to the string codes for config
std::string UpscalerToCode(Upscaler upscaler);

// Converts string codes into enum for config
Upscaler CodeToUpscaler(const std::string& code);

// Converts enum to the string codes for config
std::string SharpnessShaderToCode(SharpenShader sharpenShader);

// Converts string codes into enum for config
SharpenShader CodeToSharpnessShader(const std::string& code);
