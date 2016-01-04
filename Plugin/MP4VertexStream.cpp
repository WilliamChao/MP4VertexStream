#include "pch.h"
#include "MP4VertexStream.h"
#include "fcFoundation.h"
#include "GraphicsDevice/fcGraphicsDevice.h"



#include "Encoder/fcMP4File.h"

vsCLinkage vsExport vsIEncodeContext* fcMP4CreateContextImpl(vsEncodeConfig &conf, fcIGraphicsDevice *dev);


vsCLinkage vsExport vsIEncodeContext* vsEncodeCreateContext(vsEncodeConfig *conf)
{
    return fcMP4CreateContextImpl(*conf, fcGetGraphicsDevice());
}

vsCLinkage vsExport void vsEncodeDestroyContext(vsIEncodeContext *ctx)
{
    if (!ctx) { return; }
    ctx->release();
}

vsCLinkage vsExport void vsEncodeBeginFrame(vsIEncodeContext *ctx)
{
    // todo
}

vsCLinkage vsExport void vsEncodeAddData(vsIEncodeContext *ctx, void *pixels, vsColorSpace format)
{
    if (!ctx) { return; }
    ctx->addVideoSamples(pixels, format);
}

vsCLinkage vsExport void vsEncodeEndFrame(vsIEncodeContext *ctx)
{
    // todo
}

vsCLinkage vsExport void fcMP4ClearFrame(vsIEncodeContext *ctx)
{
    if (!ctx) { return; }
    ctx->clearFrame();
}

vsCLinkage vsExport bool vsEncodeWriteFile(vsIEncodeContext *ctx, const char *path)
{
    if (!ctx) { return false; }
    return ctx->writeFile(path);
}

vsCLinkage vsExport int vsEncodeWriteMemory(vsIEncodeContext *ctx, void *buf)
{
    if (!ctx) { return 0; }
    return ctx->writeMemory(buf);
}
