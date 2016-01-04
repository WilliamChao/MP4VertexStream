#ifndef vsAACEncoder_h
#define vsAACEncoder_h

#include <string>


class vsAACEncoder
{
public:
    struct Result
    {
        void *data;
        int size;

        Result(void *d = nullptr, int s = 0)
            : data(d), size(s) {}
    };

    static bool loadModule();

    vsAACEncoder(int sampling_rate, int num_channels, int bitrate);
    ~vsAACEncoder();
    operator bool() const;
    Result encode(const float *samples, int num_samples);
    const std::string& getHeader();

private:
    void *m_handle;
    unsigned long m_num_read_samples;
    unsigned long m_output_size;
    std::vector<float> m_tmp_buf;
    std::string m_aac_tmp_buf;
    std::string m_aac_buf;
    std::string m_aac_header;
};


#endif // vsAACEncoder_h
