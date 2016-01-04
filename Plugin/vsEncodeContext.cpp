#include "pch.h"
#include "MP4VertexStream.h"
#include "Foundation.h"
#include "vsEncodeContext.h"
#include "Encoder/vsMP4File.h"

vsEncodeContext::vsEncodeContext(const vsEncodeConfig& config)
    : m_config(config)
{

}

vsEncodeContext::~vsEncodeContext()
{
}

void vsEncodeContext::release()
{
    delete this;
}

void vsEncodeContext::beginFrame()
{

}

void vsEncodeContext::addData(void *data, vsColorSpace format)
{

}

void vsEncodeContext::endFrame()
{

}

bool vsEncodeContext::writeFile(const char *path)
{
    return m_mp4->writeFile(path);
}

int  vsEncodeContext::writeMemory(void *buf)
{
    return m_mp4->writeMemory(buf);
}
