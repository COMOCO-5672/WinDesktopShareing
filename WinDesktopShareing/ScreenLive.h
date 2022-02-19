﻿#ifndef SCREEN_LIVE_H
#define SCREEN_LIVE_H

#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <set>
#include "xop/RtspServer.h"
#include "xop/RtspPusher.h"
#include "xop/RtmpPublisher.h"
#include "capture/AudioCapture/AudioCapture.h"
#include "capture/ScreenCapture/ScreenCapture.h"
#include "codec/AACEncoder.h"
#include "codec/H264Encoder.h"


#define SCREEN_LIVE_RTSP_SERVER 1
#define SCREEN_LIVE_RTSP_PUSHER 2
#define SCREEN_LIVE_RTMP_PUSHER 3

struct AVConfig
{
    uint32_t bitrate_bps = 8000000;
    uint32_t framerate = 25;
    std::string codec = "x264";

    bool operator != (const AVConfig &src) const
    {
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

    bool is_initialized_ = false;
    bool is_capture_started_ = false;
    bool is_encoder_started_ = false;

    AVConfig av_config_;
    std::mutex mutex_;

    // capture
    ScreenCapture *screen_capture_{ nullptr };
    AudioCapture audio_capture_;

    // encoder
    H264Encoder h264_encoder_;
    AACEncoder acc_encoder_;
    std::shared_ptr<std::thread> encode_video_thread_{ nullptr };
    std::shared_ptr<std::thread> encode_audio_thread_{ nullptr };

    // stream
    xop::MediaSessionId media_session_id_ = 0;
    std::unique_ptr<xop::EventLoop> event_loop_{ nullptr };
    std::shared_ptr<xop::RtspServer> rtsp_server_{ nullptr };
    std::shared_ptr<xop::RtspPusher> rtsp_pusher_{ nullptr };
    std::shared_ptr<xop::RtmpPublisher> rtmp_pusher_{ nullptr };

    // status info
    std::atomic_int encoding_fps_;
    std::set<std::string> rtsp_clients_;

};

#endif
