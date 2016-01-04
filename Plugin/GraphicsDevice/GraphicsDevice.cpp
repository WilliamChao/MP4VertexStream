#include "pch.h"
#include "MP4VertexStream.h"
#include "Foundation.h"
#include "GraphicsDevice.h"



IGraphicsDevice* fcCreateGraphicsDeviceOpenGL(void *device);
IGraphicsDevice* fcCreateGraphicsDeviceD3D9(void *device);
IGraphicsDevice* fcCreateGraphicsDeviceD3D11(void *device);


IGraphicsDevice *g_the_graphics_device;
vsCLinkage vsExport IGraphicsDevice* fcGetGraphicsDevice() { return g_the_graphics_device; }
typedef IGraphicsDevice* (*fcGetGraphicsDeviceT)();


vsCLinkage vsExport void UnitySetGraphicsDevice(void* device, int deviceType, int eventType)
{
    if (eventType == kGfxDeviceEventInitialize) {
#ifdef vsSupportD3D9
        if (deviceType == kGfxRendererD3D9)
        {
            g_the_graphics_device = fcCreateGraphicsDeviceD3D9(device);
        }
#endif // vsSupportD3D9
#ifdef vsSupportD3D11
        if (deviceType == kGfxRendererD3D11)
        {
            g_the_graphics_device = fcCreateGraphicsDeviceD3D11(device);
        }
#endif // vsSupportD3D11
#ifdef vsSupportOpenGL
        if (deviceType == kGfxRendererOpenGL)
        {
            g_the_graphics_device = fcCreateGraphicsDeviceOpenGL(device);
        }
#endif // vsSupportOpenGL
    }

    if (eventType == kGfxDeviceEventShutdown) {
        delete g_the_graphics_device;
        g_the_graphics_device = nullptr;
    }
}

vsCLinkage vsExport void UnityRenderEvent(int)
{
}


#ifdef vsSupportOpenGL
vsCLinkage vsExport void fcInitializeOpenGL()
{
    UnitySetGraphicsDevice(nullptr, kGfxRendererOpenGL, kGfxDeviceEventInitialize);
}
#endif

#ifdef vsSupportD3D9
vsCLinkage vsExport void fcInitializeD3D9(void *device)
{
    UnitySetGraphicsDevice(device, kGfxRendererD3D9, kGfxDeviceEventInitialize);
}
#endif

#ifdef vsSupportD3D11
vsCLinkage vsExport void fcInitializeD3D11(void *device)
{
    UnitySetGraphicsDevice(device, kGfxRendererD3D11, kGfxDeviceEventInitialize);
}
#endif

vsCLinkage vsExport void fcFinalizeGraphicsDevice()
{
    UnitySetGraphicsDevice(nullptr, kGfxRendererNull, kGfxDeviceEventShutdown);
}



#if !defined(fcMaster) && defined(fcWindows)

// PatchLibrary で突っ込まれたモジュールは UnitySetGraphicsDevice() が呼ばれないので、
// DLL_PROCESS_ATTACH のタイミングで先にロードされているモジュールからデバイスをもらって同等の処理を行う。
BOOL WINAPI DllMain(HINSTANCE module_handle, DWORD reason_for_call, LPVOID reserved)
{
    if (reason_for_call == DLL_PROCESS_ATTACH)
    {
        HMODULE m = ::GetModuleHandleA("FrameCapturer.dll");
        if (m) {
            auto proc = (fcGetGraphicsDeviceT)::GetProcAddress(m, "fcGetGraphicsDevice");
            if (proc) {
                IGraphicsDevice *dev = proc();
                if (dev) {
                    UnitySetGraphicsDevice(dev->getDevicePtr(), dev->getDeviceType(), kGfxDeviceEventInitialize);
                }
            }
        }
    }
    else if (reason_for_call == DLL_PROCESS_DETACH)
    {
    }
    return TRUE;
}

// "DllMain already defined in MSVCRT.lib" 対策
#ifdef _X86_
extern "C" { int _afxForceUSRDLL; }
#else
extern "C" { int __afxForceUSRDLL; }
#endif

#endif
