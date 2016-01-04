#ifndef MP4Muxer_h
#define MP4Muxer_h

#include <iostream>

class MP4Muxer
{
public:
    struct Params
    {
        const char *out_mp4_path;
        const char *in_h264_path;
        const char *in_aac_path;
        int frame_rate;

        Params()
            : out_mp4_path(nullptr), in_h264_path(nullptr), in_aac_path(nullptr), frame_rate(0) {}
    };

    MP4Muxer();
    ~MP4Muxer();

    // equvalant to: ffmpeg -f h264 -i in_h264_path -c:v copy out_mp4_path
    bool mux(const Params &params);
    //bool mux(std::ostream &out_mp4, std::istream &in_h264, int frame_rate); // todo
};

#endif // MP4Muxer_h
