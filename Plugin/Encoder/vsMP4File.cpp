#include "pch.h"
#include "MP4VertexStream.h"

#include <libyuv/libyuv.h>
#include "Foundation.h"
#include "ThreadPool.h"
#include "GraphicsDevice/GraphicsDevice.h"
#include "vsMP4File.h"
#include "vsH264Encoder.h"
#include "vsAACEncoder.h"
#include "vsMP4Muxer.h"
#ifdef fcWindows
#pragma comment(lib, "yuv.lib")
#endif


class vsMP4Context : public vsIMP4EncodeContext
{
public:
    struct H264FrameData
    {
        std::string data;
    };

    struct RawFrameData
    {
        std::string rgba;
        uint8_t *i420_y;
        uint8_t *i420_u;
        uint8_t *i420_v;

        RawFrameData() : i420_y(nullptr), i420_u(nullptr), i420_v(nullptr) {}
        ~RawFrameData() { deallocate(); }

        void allocate(int width, int height)
        {
            deallocate();

            rgba.resize(width * height * 4);
            int af = roundup<2>(width) * roundup<2>(height);
            i420_y = (uint8_t*)aligned_alloc(af, 32);
            i420_u = (uint8_t*)aligned_alloc(af >> 2, 32);
            i420_v = (uint8_t*)aligned_alloc(af >> 2, 32);
        }

        void deallocate()
        {
            rgba.clear();
            aligned_free(i420_y);
            aligned_free(i420_u);
            aligned_free(i420_v);
        }
    };

public:
    vsMP4Context(vsEncodeConfig &conf, IGraphicsDevice *dev);
    ~vsMP4Context();
    void release() override;

    bool addVideoSamples(void *pixels, vsColorSpace cs) override;
    bool addAudioSamples(const float *samples, int num_samples) override;
    void clearFrame() override;
    bool writeFile(const char *path) override;
    int  writeMemory(void *buf) override;

private:
    void enqueueVideoTask(const std::function<void()> &f);
    void enqueueAudioTask(const std::function<void()> &f);
    void processVideoTasks();
    void processAudioTasks();

    void resetEncoders();
    void scrape(bool is_tasks_running);
    void wait();
    void waitOne();
    void addVideoFrameTask(H264FrameData &o_fdata, RawFrameData &raw_buffer, bool rgba2i420);
    void write(std::ostream &os);

private:
    vsEncodeConfig m_conf;
    IGraphicsDevice *m_dev;
    std::vector<RawFrameData> m_raw_video_buffers;
    std::vector<std::vector<float>> m_raw_audio_buffers;
    std::list<H264FrameData> m_h264_buffers;
    std::list<std::string> m_aac_buffers;
    int m_video_frame;
    int m_audio_frame;

    std::unique_ptr<vsH264Encoder> m_h264_encoder;
    std::unique_ptr<vsAACEncoder> m_aac_encoder;
    std::unique_ptr<vsMP4Muxer> m_muxer;

    std::atomic<int> m_video_active_task_count;
    std::thread m_video_worker;
    std::mutex m_video_queue_mutex;
    std::condition_variable m_video_condition;
    std::deque<std::function<void()>> m_video_tasks;

    std::atomic<int> m_audio_active_task_count;
    std::thread m_audio_worker;
    std::mutex m_audio_queue_mutex;
    std::condition_variable m_audio_condition;
    std::deque<std::function<void()>> m_audio_tasks;

    bool m_stop;
};


vsMP4Context::vsMP4Context(vsEncodeConfig &conf, IGraphicsDevice *dev)
    : m_conf(conf)
    , m_dev(dev)
    , m_video_frame(0)
    , m_audio_frame(0)
    , m_video_active_task_count(0)
    , m_audio_active_task_count(0)
    , m_stop(false)
{
    if (m_conf.video_max_buffers == 0) {
        m_conf.video_max_buffers = 4;
    }

    // allocate working buffer
    if (m_conf.video) {
        m_raw_video_buffers.resize(m_conf.video_max_buffers);
        for (auto& rf : m_raw_video_buffers)
        {
            rf.allocate(m_conf.video_width, m_conf.video_height);
        }
    }
    if (m_conf.audio) {
        m_raw_audio_buffers.resize(m_conf.video_max_buffers);
    }

    resetEncoders();
    m_muxer.reset(new vsMP4Muxer());

    // run working thread
    if (m_conf.video) {
        m_video_worker = std::thread([this](){ processVideoTasks(); });
    }
    if (m_conf.audio) {
        m_audio_worker = std::thread([this](){ processAudioTasks(); });
    }
}

vsMP4Context::~vsMP4Context()
{
    // stop working thread
    m_stop = true;
    if (m_conf.video) {
        m_video_condition.notify_all();
        m_video_worker.join();
    }
    if (m_conf.audio) {
        m_audio_condition.notify_all();
        m_audio_worker.join();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void vsMP4Context::resetEncoders()
{
    wait();

    m_h264_encoder.reset();
    if (m_conf.video) {
        m_h264_encoder.reset(new vsH264Encoder(m_conf.video_width, m_conf.video_height, m_conf.video_framerate, m_conf.video_bitrate));
    }

    m_aac_encoder.reset();
    if (m_conf.audio) {
        m_aac_encoder.reset(new vsAACEncoder(m_conf.audio_sampling_rate, m_conf.audio_num_channels, m_conf.audio_bitrate));
    }
}

void vsMP4Context::enqueueVideoTask(const std::function<void()> &f)
{
    {
        std::unique_lock<std::mutex> lock(m_video_queue_mutex);
        m_video_tasks.push_back(std::function<void()>(f));
    }
    m_video_condition.notify_one();
}

void vsMP4Context::enqueueAudioTask(const std::function<void()> &f)
{
    {
        std::unique_lock<std::mutex> lock(m_audio_queue_mutex);
        m_audio_tasks.push_back(std::function<void()>(f));
    }
    m_audio_condition.notify_one();
}

void vsMP4Context::processVideoTasks()
{
    while (!m_stop)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_video_queue_mutex);
            while (!m_stop && m_video_tasks.empty()) {
                m_video_condition.wait(lock);
            }
            if (m_stop) { return; }

            task = m_video_tasks.front();
            m_video_tasks.pop_front();
        }
        task();
    }
}

void vsMP4Context::processAudioTasks()
{
    while (!m_stop)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_audio_queue_mutex);
            while (!m_stop && m_audio_tasks.empty()) {
                m_audio_condition.wait(lock);
            }
            if (m_stop) { return; }

            task = m_audio_tasks.front();
            m_audio_tasks.pop_front();
        }
        task();
    }
}


void vsMP4Context::release()
{
    delete this;
}

void vsMP4Context::scrape(bool updating)
{
    // 最大容量か最大フレーム数が設定されている場合、それらを超過したフレームをここで切り捨てる。
    // 切り捨てるフレームがパレットを持っている場合パレットの移動も行う。

    // 実行中のタスクが更新中のデータを間引くのはマズいので、更新中は最低でもタスク数分は残す
    int min_frames = updating ? std::max<int>(m_conf.video_max_buffers, 1) : 1;

    // todo
}

void vsMP4Context::addVideoFrameTask(H264FrameData &o_fdata, RawFrameData &raw, bool rgba2i420)
{
    // 必要であれば RGBA -> I420 変換
    int width = m_conf.video_width;
    int frame_size = m_conf.video_width * m_conf.video_height;
    uint8 *y = raw.i420_y;
    uint8 *u = raw.i420_u;
    uint8 *v = raw.i420_v;
    if (rgba2i420) {
        libyuv::ABGRToI420(
            (uint8*)&raw.rgba[0], width * 4,
            y, width,
            u, width >> 1,
            v, width >> 1,
            m_conf.video_width, m_conf.video_height );
    }

    // I420 のピクセルデータを H264 へエンコード
    auto ret = m_h264_encoder->encodeI420(y, u, v);
    o_fdata.data.assign((char*)ret.data, ret.size);
}

void vsMP4Context::wait()
{
    while (m_video_active_task_count > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void vsMP4Context::waitOne()
{
    // 実行中のタスクの数が上限に達している場合適当に待つ
    while (m_video_active_task_count >= m_conf.video_max_buffers)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool vsMP4Context::addVideoSamples(void *pixels, vsColorSpace cs)
{
    if (!m_h264_encoder) { return false; }
    waitOne();
    int frame = m_video_frame++;
    RawFrameData& raw = m_raw_video_buffers[frame % m_conf.video_max_buffers];

    bool rgba2i420 = true;
    if (cs == vsColorSpace_RGBA) {
        raw.rgba.assign((char*)pixels, raw.rgba.size());
    }
    else if (cs == vsColorSpace_I420) {
        rgba2i420 = false;

        int frame_size = m_conf.video_width * m_conf.video_height;
        const uint8_t *src_y = (const uint8_t*)pixels;
        const uint8_t *src_u = src_y + frame_size;
        const uint8_t *src_v = src_u + (frame_size >> 2);
        memcpy(raw.i420_y, src_y, frame_size);
        memcpy(raw.i420_u, src_u, frame_size >> 2);
        memcpy(raw.i420_v, src_v, frame_size >> 2);
    }

    // h264 データを生成
    m_h264_buffers.push_back(H264FrameData());
    H264FrameData& fdata = m_h264_buffers.back();
    ++m_video_active_task_count;
    enqueueVideoTask([this, &fdata, &raw, rgba2i420](){
        addVideoFrameTask(fdata, raw, rgba2i420);
        --m_video_active_task_count;
    });

    scrape(true);
    return true;
}

bool vsMP4Context::addAudioSamples(const float *samples, int num_samples)
{
    if (!m_aac_encoder) { return false; }
    waitOne();

    int frame = m_audio_frame++;
    auto& raw = m_raw_audio_buffers[frame % m_conf.video_max_buffers];
    raw.assign(samples, samples+num_samples);

    m_aac_buffers.push_back(std::string());
    auto& aacdata = m_aac_buffers.back();

    // aac encode
    ++m_audio_active_task_count;
    enqueueVideoTask([this, &aacdata, &raw](){
        auto ret = m_aac_encoder->encode(&raw[0], raw.size());
        aacdata.assign((char*)ret.data, ret.size);
        --m_audio_active_task_count;
    });

    return true;
}

void vsMP4Context::clearFrame()
{
    wait();
    m_video_frame = 0;
    m_audio_frame = 0;
    m_h264_buffers.clear();
    m_aac_buffers.clear();
    resetEncoders();
}


static inline void adjust_frame(int &begin_frame, int &end_frame, int max_frame)
{
    begin_frame = std::max<int>(begin_frame, 0);
    if (end_frame < 0) {
        end_frame = max_frame;
    }
    else {
        end_frame = std::min<int>(end_frame, max_frame);
    }
}


void vsMP4Context::write(std::ostream &os)
{
    addAudioSamples(nullptr, 0); // flush
    wait();
    scrape(false);

    char tmp_h264_filename[256];
    char tmp_aac_filename[256];
    char tmp_mp4_filename[256];

    uint64_t now = (uint64_t)::time(nullptr);
    sprintf(tmp_h264_filename, "%llu.h264", now);
    sprintf(tmp_aac_filename, "%llu.aac", now);
    sprintf(tmp_mp4_filename, "%llu.mp4", now);

    vsMP4Muxer::Params params;
    params.frame_rate = m_conf.video_framerate;
    params.out_mp4_path = tmp_mp4_filename;
    if(m_conf.video) {
        params.in_h264_path = tmp_h264_filename;
        std::ofstream tmp_h264(tmp_h264_filename, std::ios::binary);

        auto begin = m_h264_buffers.begin();
        auto end = m_h264_buffers.end();
        for (auto i = begin; i != end; ++i) {
            tmp_h264.write(&i->data[0], i->data.size());
        }
    }
    if (m_conf.audio) {
        params.in_aac_path = tmp_aac_filename;
        std::ofstream tmp_aac(tmp_aac_filename, std::ios::binary);

        for (const auto& aac : m_aac_buffers) {
            tmp_aac.write(&aac[0], aac.size());
        }
    }

    m_muxer->mux(params);

    {
        char buf[1024];
        std::ifstream tmp_mp4(tmp_mp4_filename, std::ios::binary);
        while (!tmp_mp4.eof()) {
            tmp_mp4.read(buf, sizeof(buf));
            os.write(buf, tmp_mp4.gcount());
        }
    }
#ifdef fcMaster
    std::remove(tmp_aac_filename);
    std::remove(tmp_h264_filename);
    std::remove(tmp_mp4_filename);
#endif // fcMaster
}

bool vsMP4Context::writeFile(const char *path)
{
    std::ofstream os(path, std::ios::binary);
    if (!os) { return false; }
    write(os);
    os.close();
    return true;
}

int vsMP4Context::writeMemory(void *buf)
{
    std::ostringstream os(std::ios::binary);
    write(os);

    std::string s = os.str();
    memcpy(buf, &s[0], s.size());
    return (int)s.size();
}


vsCLinkage vsExport vsIMP4EncodeContext* vsMP4EncodeCreateContextImpl(vsEncodeConfig &conf, IGraphicsDevice *dev)
{
    if (vsH264Encoder::loadModule()) {
        return new vsMP4Context(conf, dev);
    }
    return nullptr;
}
