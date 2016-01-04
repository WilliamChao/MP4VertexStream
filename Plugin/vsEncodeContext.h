#ifndef vsEncodeContext_h
#define vsEncodeContext_h

class IMP4EncodeContext;


class vsEncodeContext
{
public:
    vsEncodeContext(const vsEncodeConfig& config);
protected:
    ~vsEncodeContext();

public:
    void release();

    void beginFrame();
    void addData(const void *data, int num_elements, vsDataFormat format);
    void endFrame();

    bool writeFile(const char *path);
    int  writeMemory(void *buf);

private:
    vsEncodeConfig m_config;
    IMP4EncodeContext *m_mp4;
    std::vector<char> m_buf;
    int m_buf_pos;
};

#endif // vsEncodeContext_h
