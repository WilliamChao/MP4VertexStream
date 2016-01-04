#include "pch.h"
#include "MP4VertexStream.h"
#include "Foundation.h"
#include "GraphicsDevice/GraphicsDevice.h"
#include "vsEncodeContext.h"
#include "vsDecodeContext.h"



#include "Encoder/MP4File.h"

vsCLinkage vsExport vsEncodeContext* vsMP4EncodeCreateContextImpl(vsEncodeConfig &conf, IGraphicsDevice *dev);


vsCLinkage vsExport vsEncodeContext* vsEncodeCreateContext(vsEncodeConfig *conf)
{
    return vsMP4EncodeCreateContextImpl(*conf, fcGetGraphicsDevice());
}

vsCLinkage vsExport void vsEncodeDestroyContext(vsEncodeContext *ctx)
{
    if (!ctx) { return; }
    ctx->release();
}

vsCLinkage vsExport void vsEncodeBeginFrame(vsEncodeContext *ctx)
{
    ctx->beginFrame();
}

vsCLinkage vsExport void vsEncodeAddData(vsEncodeContext *ctx, void *pixels, int num_elements, vsDataFormat format)
{
    if (!ctx) { return; }
    ctx->addData(pixels, num_elements, format);
}

vsCLinkage vsExport void vsEncodeEndFrame(vsEncodeContext *ctx)
{
    ctx->endFrame();
}

vsCLinkage vsExport bool vsEncodeWriteFile(vsEncodeContext *ctx, const char *path)
{
    if (!ctx) { return false; }
    return ctx->writeFile(path);
}

vsCLinkage vsExport int vsEncodeWriteMemory(vsEncodeContext *ctx, void *buf)
{
    if (!ctx) { return 0; }
    return ctx->writeMemory(buf);
}




vsCLinkage vsExport int vsGetDataSize(vsDataFormat fmt)
{
    static const int s_table[] = {
        0, // vsDataFormat_Unknown,
        4, // vsDataFormat_RGBA8,
        3, // vsDataFormat_RGB8,
        2, // vsDataFormat_RG8,
        1, // vsDataFormat_R8,
        8, // vsDataFormat_RGBAHalf,
        6, // vsDataFormat_RGBHalf,
        4, // vsDataFormat_RGHalf,
        2, // vsDataFormat_RHalf,
        16,// vsDataFormat_RGBAFloat,
        12,// vsDataFormat_RGBFloat,
        8, // vsDataFormat_RGFloat,
        4, // vsDataFormat_RFloat,
        16,// vsDataFormat_RGBAInt,
        12,// vsDataFormat_RGBInt,
        8, // vsDataFormat_RGInt,
        4, // vsDataFormat_RInt,
    };
    return s_table[(int)fmt];
}

//vsCLinkage vsExport void vsConvertData(void *out_buf, vsDataFormat out_fmt, const void *in_buf, vsDataFormat in_fmt)
//{
//
//}
