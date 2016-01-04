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
    m_mp4 = vsMP4EncodeCreateContext(config);
    m_buf.reserve(config.video_width * config.video_height * 4);
}

vsEncodeContext::~vsEncodeContext()
{
    if (m_mp4 != nullptr) {
        m_mp4->release();
        m_mp4 = nullptr;
    }
}

void vsEncodeContext::beginFrame()
{

}

void vsEncodeContext::addData(const void *data_, int num_elements, vsDataFormat format)
{
    const char *data = (const char*)data_;
    int data_size = num_elements * vsGetDataSize(format);
    // todo: convert 
    memcpy(&m_buf[m_buf_pos], data, data_size);
    m_buf_pos += data_size;
}

void vsEncodeContext::endFrame()
{
    if (m_mp4 != nullptr) {
        m_mp4->addVideoSamples(&m_buf[0], vsColorSpace_RGBA);
    }
    m_buf_pos = 0;
}

bool vsEncodeContext::writeFile(const char *path)
{
    if (m_mp4 != nullptr) {
        return m_mp4->writeFile(path);
    }
    return false;
}

int  vsEncodeContext::writeMemory(void *buf)
{
    if (m_mp4 != nullptr) {
        return m_mp4->writeMemory(buf);
    }
    return 0;
}
