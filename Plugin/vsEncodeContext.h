#ifndef vsEncodeContext_h
#define vsEncodeContext_h

class vsIMP4EncodeContext;


class vsEncodeContext
{
public:
    vsEncodeContext(const vsEncodeConfig& config);
protected:
    ~vsEncodeContext();

public:
    void release();

    void beginFrame();
    void addData(void *data, vsColorSpace format = vsColorSpace_RGBA);
    void endFrame();

    bool writeFile(const char *path);
    int  writeMemory(void *buf);

private:
    vsEncodeConfig m_config;
    vsIMP4EncodeContext *m_mp4;
    std::vector<char> m_buf;
};

#endif // vsEncodeContext_h
