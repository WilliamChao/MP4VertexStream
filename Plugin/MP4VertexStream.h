#ifndef FrameCapturer_h
#define FrameCapturer_h

//options:
//#define fcSupportMP4
//#define fcSupportOpenGL
//#define fcSupportD3D9
//#define fcSupportD3D11
//#define fcWithTBB


#if defined(_WIN32)
    #define fcWindows
#elif defined(__APPLE__)
    #ifdef TARGET_OS_IPHONE
        #define fciOS
    #else
        #define fcMac
    #endif
#elif defined(__ANDROID__)
    #define fcAndroid
#elif defined(__linux__)
    #define fcLinux
#endif

#ifdef _MSC_VER
    #define fcMSVC
#endif 


#define vsCLinkage extern "C"
#ifdef fcWindows
    #define vsExport __declspec(dllexport)
    #define fcBreak() DebugBreak()
#else // fcWindows
    #define vsExport
    #define fcBreak() __builtin_trap()
#endif // fcWindows

#ifdef fcDebug
    void fcDebugLogImpl(const char* fmt, ...);
    #define fcDebugLog(...) fcDebugLogImpl(__VA_ARGS__)
    #ifdef fcVerboseDebug
        #define fcDebugLogVerbose(...) fcDebugLogImpl(__VA_ARGS__)
    #else
        #define fcDebugLogVerbose(...)
    #endif
#else
    #define fcDebugLog(...)
    #define fcDebugLogVerbose(...)
#endif



class IGraphicsDevice;
class vsIEncodeContext;
class vsIDecodeContext;

enum vsColorSpace
{
    vsColorSpace_Unknown,
    vsColorSpace_RGBA,
    vsColorSpace_I420,
};

enum vsPixelFormat
{
    vsPixelFormat_Unknown,
    vsPixelFormat_RGBA8,
    vsPixelFormat_RGB8,
    vsPixelFormat_RG8,
    vsPixelFormat_R8,
    vsPixelFormat_RGBAHalf,
    vsPixelFormat_RGBHalf,
    vsPixelFormat_RGHalf,
    vsPixelFormat_RHalf,
    vsPixelFormat_RGBAFloat,
    vsPixelFormat_RGBFloat,
    vsPixelFormat_RGFloat,
    vsPixelFormat_RFloat,
    vsPixelFormat_RGBAInt,
    vsPixelFormat_RGBInt,
    vsPixelFormat_RGInt,
    vsPixelFormat_RInt,
};

enum fcETextureFormat
{
    fcE_ARGB32 = 0,
    fcE_Depth = 1,
    fcE_ARGBHalf = 2,
    fcE_Shadowmap = 3,
    fcE_RGB565 = 4,
    fcE_ARGB4444 = 5,
    fcE_ARGB1555 = 6,
    fcE_Default = 7,
    fcE_ARGB2101010 = 8,
    fcE_DefaultHDR = 9,
    fcE_ARGBFloat = 11,
    fcE_RGFloat = 12,
    fcE_RGHalf = 13,
    fcE_RFloat = 14,
    fcE_RHalf = 15,
    fcE_R8 = 16,
    fcE_ARGBInt = 17,
    fcE_RGInt = 18,
    fcE_RInt = 19,
};



struct vsEncodeConfig
{
    int video; // bool
    int audio; // bool
    int video_width;
    int video_height;
    int video_bitrate;
    int video_framerate;
    int video_max_buffers;
    int video_max_frame;
    int video_max_data_size;
    int audio_sampling_rate;
    int audio_num_channels;
    int audio_bitrate;

    vsEncodeConfig()
        : video(true), audio(false)
        , video_width(320), video_height(240)
        , video_bitrate(256000), video_framerate(30)
        , video_max_buffers(8), video_max_frame(0), video_max_data_size(0)
        , audio_sampling_rate(48000), audio_num_channels(2), audio_bitrate(64000)
    {}
};

struct vsDecodeConfig
{

};

vsCLinkage vsExport vsIEncodeContext*   vsEncodeCreateContext(vsEncodeConfig *conf);
vsCLinkage vsExport void                vsEncodeDestroyContext(vsIEncodeContext *ctx);
vsCLinkage vsExport void                vsEncodeBeginFrame(vsIEncodeContext *ctx);
vsCLinkage vsExport void                vsEncodeAddData(vsIEncodeContext *ctx, void *data, vsColorSpace format = vsColorSpace_RGBA);
vsCLinkage vsExport void                vsEncodeEndFrame(vsIEncodeContext *ctx);
vsCLinkage vsExport bool                vsEncodeWriteFile(vsIEncodeContext *ctx, const char *path);
vsCLinkage vsExport int                 vsEncodeWriteMemory(vsIEncodeContext *ctx, void *buf);

vsCLinkage vsExport vsIDecodeContext*   vsDecodeCreateContext(vsDecodeConfig *conf);
vsCLinkage vsExport void                vsDecodeDestroyContext(vsIDecodeContext *ctx);
vsCLinkage vsExport void                vsDecodeBeginFrame(vsIDecodeContext *ctx);
vsCLinkage vsExport void                vsDecodeGetData(vsIDecodeContext *ctx, void *data, vsColorSpace format = vsColorSpace_RGBA);
vsCLinkage vsExport void                vsDecodeEndFrame(vsIDecodeContext *ctx);
vsCLinkage vsExport bool                vsDecodeReadFile(vsIDecodeContext *ctx, const char *path);
vsCLinkage vsExport int                 vsDecodeReadMemory(vsIDecodeContext *ctx, void *buf);

#endif // FrameCapturer_h
