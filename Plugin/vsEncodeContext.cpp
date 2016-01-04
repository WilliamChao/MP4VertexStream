#include "pch.h"
#include "MP4VertexStream.h"
#include "Foundation.h"
#include "vsEncodeContext.h"
#include "Encoder/MP4File.h"

vsEncodeContext::vsEncodeContext(const vsEncodeConfig& config)
    : m_config(config)
    , m_mp4(nullptr)
    , m_buf()
    , m_buf_pos(0)
{
    m_buf.reserve(config.video_width * config.video_height * 4);
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

void vsEncodeContext::addData(const void *data_, int num_elements, vsDataFormat format)
{
    const char *data = (const char*)data_;
    int data_size = num_elements * vsGetDataSize(format);
    m_buf.insert(m_buf.end(), data, data + data_size);
    m_buf_pos += data_size;
}

void vsEncodeContext::endFrame()
{
    m_mp4->addVideoSamples(&m_buf[0], vsColorSpace_RGBA);
    m_buf_pos = 0;
}

bool vsEncodeContext::writeFile(const char *path)
{
    return m_mp4->writeFile(path);
}

int  vsEncodeContext::writeMemory(void *buf)
{
    return m_mp4->writeMemory(buf);
}
