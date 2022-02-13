#ifndef SCREEN_LIVE_H
#define SCREEN_LIVE_H

#include <vector>
#include <string>
#include <atomic>
#include <mutex>

struct AVConfig
{
    uint32_t bitrate_bps = 8000000;
    uint32_t framerate = 25;
    std::string codec = "x264";

    bool operator != (const AVConfig &src) const {
        if (src.bitrate_bps != bitrate_bps
            || src.framerate != framerate
            || src.codec != codec) {
            return true;
        }
        return false;
    }
};

struct LiveConfig
{
    //pusher
    std::string     rtsp_url;
    std::string     rtmp_url;

    //server
    std::string     suffix;
    std::string     ip;
    uint16_t        port;
};



class ScreenLive {
public:
    ScreenLive &operator = (const ScreenLive &) = delete;
    ScreenLive(const ScreenLive &) = delete;
    static ScreenLive &Instance();
    ~ScreenLive();

    bool Init(AVConfig &config);
    void Destory();
    bool IsInitialized() { return m_bInited_; }

    int StartCapture();
    int StopCapture();

    int StartEncoder(AVConfig &config);
    int StopEncoder();
    bool IsEncoderInited() { return m_bEncoderInited_; };

    bool StartLive(int type, LiveConfig &config);
    void StopLive(int type);
    bool IsConnected(int type);

    bool GetScreenImage(std::vector<uint8_t>& bgra_image, uint32_t &width, uint32_t &height);

    std::string GetStatusInfo();

private:
    ScreenLive();

    void EncoderVideo();
    void EncoderAudio();
    void PushVideo(const uint8_t*data, uint32_t size, uint32_t timestamp);
    void PushAudio(const uint8_t *data, uint32_t size, uint32_t timestamp);
    bool IsKeyFrame(const uint8_t *data, uint32_t size);

private:
    std::atomic_bool m_bInited_{ false };
    std::atomic_bool m_bEncoderInited_{ false };

    AVConfig m_sAvconfig_;
    std::mutex m_mtx_;

};

#endif
