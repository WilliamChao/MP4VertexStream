class vsIMP4EncodeContext
{
public:
    virtual void release() = 0;

    virtual bool addVideoSamples(void *pixels, vsColorSpace cs) = 0;
    virtual bool addAudioSamples(const float *samples, int num_samples) = 0;
    virtual void clearFrame() = 0;
    virtual bool writeFile(const char *path) = 0;
    virtual int  writeMemory(void *buf) = 0;

protected:
    virtual ~vsIMP4EncodeContext() {}
};

typedef vsIMP4EncodeContext* (*vsMP4EncodeCreateContextImplT)(vsEncodeConfig &conf, IGraphicsDevice*);
