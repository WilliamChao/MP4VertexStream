﻿class IMP4EncodeContext
{
public:
    virtual void release() = 0;

    virtual bool addVideoSamples(void *pixels, vsColorSpace cs) = 0;
    virtual bool addAudioSamples(const float *samples, int num_samples) = 0;
    virtual void clearFrame() = 0;
    virtual bool writeFile(const char *path) = 0;
    virtual int  writeMemory(void *buf) = 0;

protected:
    virtual ~IMP4EncodeContext() {}
};

IMP4EncodeContext* vsMP4EncodeCreateContext(const vsEncodeConfig &conf);
