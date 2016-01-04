#include "pch.h"
#include "MP4VertexStream.h"
#include "Foundation.h"
#include "GraphicsDevice/GraphicsDevice.h"
#include "vsEncodeContext.h"
#include "vsDecodeContext.h"



#include "Encoder/vsMP4File.h"

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

vsCLinkage vsExport void vsEncodeAddData(vsEncodeContext *ctx, void *pixels, vsColorSpace format)
{
    if (!ctx) { return; }
    ctx->addData(pixels, format);
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
