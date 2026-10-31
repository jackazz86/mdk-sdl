/* MDK: the SDL3 calls of the managed game that kg_sdl_exports.c lacks. No SDL headers on purpose:
   the declarations are weak, so a function this SDL build does not have resolves to NULL
   (EntryPointNotFound when the game calls it) instead of failing the link. */
#include <string.h>

extern void SDL_BlitGPUTexture(void) __attribute__((weak));
extern void SDL_CloseGamepad(void) __attribute__((weak));
extern void SDL_CloseSensor(void) __attribute__((weak));
extern void SDL_CreateColorCursor(void) __attribute__((weak));
extern void SDL_DelayPrecise(void) __attribute__((weak));
extern void SDL_DestroyCursor(void) __attribute__((weak));
extern void SDL_GPUTextureSupportsFormat(void) __attribute__((weak));
extern void SDL_GPUTextureSupportsSampleCount(void) __attribute__((weak));
extern void SDL_GenerateMipmapsForGPUTexture(void) __attribute__((weak));
extern void SDL_GetCurrentDisplayOrientation(void) __attribute__((weak));
extern void SDL_GetCurrentVideoDriver(void) __attribute__((weak));
extern void SDL_GetDesktopDisplayMode(void) __attribute__((weak));
extern void SDL_GetDisplayUsableBounds(void) __attribute__((weak));
extern void SDL_GetDisplays(void) __attribute__((weak));
extern void SDL_GetFullscreenDisplayModes(void) __attribute__((weak));
extern void SDL_GetGPUDeviceProperties(void) __attribute__((weak));
extern void SDL_GetGamepadAxis(void) __attribute__((weak));
extern void SDL_GetGamepadButton(void) __attribute__((weak));
extern void SDL_GetGamepadName(void) __attribute__((weak));
extern void SDL_GetGamepads(void) __attribute__((weak));
extern void SDL_GetKeyFromScancode(void) __attribute__((weak));
extern void SDL_GetPerformanceCounter(void) __attribute__((weak));
extern void SDL_GetPerformanceFrequency(void) __attribute__((weak));
extern void SDL_GetPrimaryDisplay(void) __attribute__((weak));
extern void SDL_GetScancodeFromName(void) __attribute__((weak));
extern void SDL_GetScancodeName(void) __attribute__((weak));
extern void SDL_GetSensorTypeForID(void) __attribute__((weak));
extern void SDL_GetSensors(void) __attribute__((weak));
extern void SDL_GetTicks(void) __attribute__((weak));
extern void SDL_GetTicksNS(void) __attribute__((weak));
extern void SDL_GetVersion(void) __attribute__((weak));
extern void SDL_GetWindowRelativeMouseMode(void) __attribute__((weak));
extern void SDL_HideCursor(void) __attribute__((weak));
extern void SDL_IOFromConstMem(void) __attribute__((weak));
extern void SDL_LoadPNG_IO(void) __attribute__((weak));
extern void SDL_OpenGamepad(void) __attribute__((weak));
extern void SDL_OpenSensor(void) __attribute__((weak));
extern void SDL_PushEvent(void) __attribute__((weak));
extern void SDL_QuitSubSystem(void) __attribute__((weak));
extern void SDL_SetCursor(void) __attribute__((weak));
extern void SDL_SetWindowFullscreenMode(void) __attribute__((weak));
extern void SDL_SetWindowRelativeMouseMode(void) __attribute__((weak));
extern void SDL_ShowCursor(void) __attribute__((weak));

extern void SDL_GetWindowSafeArea(void) __attribute__((weak));
extern void SDL_SetNumberProperty(void) __attribute__((weak));
extern void SDL_StartTextInputWithProperties(void) __attribute__((weak));
extern void SDL_TextInputActive(void) __attribute__((weak));

void *mdk_getsym_SDL3(const char *name)
{
    if (!strcmp(name, "SDL_BlitGPUTexture")) return (void *)SDL_BlitGPUTexture;
    if (!strcmp(name, "SDL_CloseGamepad")) return (void *)SDL_CloseGamepad;
    if (!strcmp(name, "SDL_CloseSensor")) return (void *)SDL_CloseSensor;
    if (!strcmp(name, "SDL_CreateColorCursor")) return (void *)SDL_CreateColorCursor;
    if (!strcmp(name, "SDL_DelayPrecise")) return (void *)SDL_DelayPrecise;
    if (!strcmp(name, "SDL_DestroyCursor")) return (void *)SDL_DestroyCursor;
    if (!strcmp(name, "SDL_GPUTextureSupportsFormat")) return (void *)SDL_GPUTextureSupportsFormat;
    if (!strcmp(name, "SDL_GPUTextureSupportsSampleCount")) return (void *)SDL_GPUTextureSupportsSampleCount;
    if (!strcmp(name, "SDL_GenerateMipmapsForGPUTexture")) return (void *)SDL_GenerateMipmapsForGPUTexture;
    if (!strcmp(name, "SDL_GetCurrentDisplayOrientation")) return (void *)SDL_GetCurrentDisplayOrientation;
    if (!strcmp(name, "SDL_GetCurrentVideoDriver")) return (void *)SDL_GetCurrentVideoDriver;
    if (!strcmp(name, "SDL_GetDesktopDisplayMode")) return (void *)SDL_GetDesktopDisplayMode;
    if (!strcmp(name, "SDL_GetDisplayUsableBounds")) return (void *)SDL_GetDisplayUsableBounds;
    if (!strcmp(name, "SDL_GetDisplays")) return (void *)SDL_GetDisplays;
    if (!strcmp(name, "SDL_GetFullscreenDisplayModes")) return (void *)SDL_GetFullscreenDisplayModes;
    if (!strcmp(name, "SDL_GetGPUDeviceProperties")) return (void *)SDL_GetGPUDeviceProperties;
    if (!strcmp(name, "SDL_GetGamepadAxis")) return (void *)SDL_GetGamepadAxis;
    if (!strcmp(name, "SDL_GetGamepadButton")) return (void *)SDL_GetGamepadButton;
    if (!strcmp(name, "SDL_GetGamepadName")) return (void *)SDL_GetGamepadName;
    if (!strcmp(name, "SDL_GetGamepads")) return (void *)SDL_GetGamepads;
    if (!strcmp(name, "SDL_GetKeyFromScancode")) return (void *)SDL_GetKeyFromScancode;
    if (!strcmp(name, "SDL_GetPerformanceCounter")) return (void *)SDL_GetPerformanceCounter;
    if (!strcmp(name, "SDL_GetPerformanceFrequency")) return (void *)SDL_GetPerformanceFrequency;
    if (!strcmp(name, "SDL_GetPrimaryDisplay")) return (void *)SDL_GetPrimaryDisplay;
    if (!strcmp(name, "SDL_GetScancodeFromName")) return (void *)SDL_GetScancodeFromName;
    if (!strcmp(name, "SDL_GetScancodeName")) return (void *)SDL_GetScancodeName;
    if (!strcmp(name, "SDL_GetSensorTypeForID")) return (void *)SDL_GetSensorTypeForID;
    if (!strcmp(name, "SDL_GetSensors")) return (void *)SDL_GetSensors;
    if (!strcmp(name, "SDL_GetTicks")) return (void *)SDL_GetTicks;
    if (!strcmp(name, "SDL_GetTicksNS")) return (void *)SDL_GetTicksNS;
    if (!strcmp(name, "SDL_GetVersion")) return (void *)SDL_GetVersion;
    if (!strcmp(name, "SDL_GetWindowRelativeMouseMode")) return (void *)SDL_GetWindowRelativeMouseMode;
    if (!strcmp(name, "SDL_HideCursor")) return (void *)SDL_HideCursor;
    if (!strcmp(name, "SDL_IOFromConstMem")) return (void *)SDL_IOFromConstMem;
    if (!strcmp(name, "SDL_LoadPNG_IO")) return (void *)SDL_LoadPNG_IO;
    if (!strcmp(name, "SDL_OpenGamepad")) return (void *)SDL_OpenGamepad;
    if (!strcmp(name, "SDL_OpenSensor")) return (void *)SDL_OpenSensor;
    if (!strcmp(name, "SDL_PushEvent")) return (void *)SDL_PushEvent;
    if (!strcmp(name, "SDL_QuitSubSystem")) return (void *)SDL_QuitSubSystem;
    if (!strcmp(name, "SDL_SetCursor")) return (void *)SDL_SetCursor;
    if (!strcmp(name, "SDL_SetWindowFullscreenMode")) return (void *)SDL_SetWindowFullscreenMode;
    if (!strcmp(name, "SDL_SetWindowRelativeMouseMode")) return (void *)SDL_SetWindowRelativeMouseMode;
    if (!strcmp(name, "SDL_ShowCursor")) return (void *)SDL_ShowCursor;
    if (!strcmp(name, "SDL_GetWindowSafeArea")) return (void *)SDL_GetWindowSafeArea;
    if (!strcmp(name, "SDL_SetNumberProperty")) return (void *)SDL_SetNumberProperty;
    if (!strcmp(name, "SDL_StartTextInputWithProperties")) return (void *)SDL_StartTextInputWithProperties;
    if (!strcmp(name, "SDL_TextInputActive")) return (void *)SDL_TextInputActive;
    return NULL;
}
