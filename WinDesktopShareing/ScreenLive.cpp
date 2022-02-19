#include "ScreenLive.h"
#include "xop/RtspServer.h"
#include "xop/H264Parser.h"
#include "capture/ScreenCapture/DXGIScreenCapture.h"
#include "capture/ScreenCapture/GDIScreenCapture.h"
#include "VersionHelpers.h"

ScreenLive::ScreenLive() : event_loop_(new xop::EventLoop())
{
    encoding_fps_ = 0;
    rtsp_clients_.clear();
}

ScreenLive::~ScreenLive()
{
    Destory();
}

ScreenLive &ScreenLive::Instance()
{
    static ScreenLive s_screen_live;
    return s_screen_live;
}

bool ScreenLive::GetScreenImage(std::vector<uint8_t>& bgra_image, uint32_t &width, uint32_t &height)
{
    if (screen_capture_) {
        if (screen_capture_->CaptureFrame(bgra_image, width, height))
            return true;
    }

    return false;
}


std::string ScreenLive::GetStatusInfo()
{
    std::string info;

    if (is_encoder_started_) {
        info += "Encoder: " + av_config_.codec + " \n\n";
        info += "Encoding framerate: " + std::to_string(encoding_fps_) + " \n\n";
    }

    if (rtsp_server_ != nullptr) {
        info + "RTSP Server (connections): " + std::to_string(rtsp_clients_.size()) + " \n\n";
    }

    if (rtsp_pusher_ != nullptr) {
        std::string status = rtsp_pusher_->IsConnected() ? "connected" : "disconnected";
        info += "RTSP Pusher: " + status + " \n\n";
    }

    if (rtmp_pusher_ != nullptr) {
        std::string status = rtmp_pusher_->IsConnected() ? "connected" : "disconnected";
        info += "RTMP Pusher: " + status + " \n\n";
    }

    return info;
}

bool ScreenLive::Init(AVConfig& config)
{
    if (is_initialized_)
        Destory();

    if (StartCapture() < 0)
        return false;

    if (StartEncoder(config) < 0)
        return false;

    is_initialized_ = true;
    return true;
}

void ScreenLive::Destory()
{
    {
        std::lock_guard<std::mutex> locker(mutex_);
        if (rtsp_pusher_ != nullptr && rtsp_pusher_->IsConnected()) {
            rtsp_pusher_->Close();
            rtmp_pusher_ = nullptr;
        }

        if (rtmp_pusher_ != nullptr && rtmp_pusher_->IsConnected()) {
            rtmp_pusher_->Close();
            rtmp_pusher_ = nullptr;
        }

        if (rtsp_server_ != nullptr) {
            rtsp_server_->RemoveSession(media_session_id_);
            rtsp_server_ = nullptr;
        }
    }

    StopEncoder();
    StopCapture();
    is_initialized_ = false;
}


bool ScreenLive::StartLive(int type, LiveConfig& config)
{
    if (!is_encoder_started_)
        return false;

    uint32_t samplerate = audio_capture_.GetSamplerate();
    uint32_t channels = audio_capture_.GetChannels();

    if (type == SCREEN_LIVE_RTSP_SERVER) {
        auto rtsp_server = xop::RtspServer::Create(event_loop_.get());
        xop::MediaSessionId session_id = 0;

        if (config.ip == "127.0.0.1")
            config.ip = "0.0.0.0";

        if (!rtsp_server->Start(config.ip, config.port))
            return false;

        xop::MediaSession *session = xop::MediaSession::CreateNew(config.suffix);
        session->AddSource(xop::channel_0, xop::H264Source::CreateNew());
        session->AddSource(xop::channel_1, xop::AACSource::CreateNew(samplerate, channels, false));
        session->AddNotifyConnectedCallback([this](xop::MediaSessionId, std::string peer_ip, uint16_t peer_port) {
            this->rtsp_clients_.emplace(peer_ip + ":" + std::to_string(peer_port));
            printf("RTSP client: %u\n", this->rtsp_clients_.size());
        });
        session->AddNotifyDisconnectedCallback([this](xop::MediaSessionId, std::string peer_ip, uint16_t peer_port) {
            this->rtsp_clients_.erase(peer_ip + ":" + std::to_string(peer_port));
            printf("RTSP Client: %u\n", this->rtsp_clients_.size());
        });

        session_id = rtsp_server->AddSession(session);
        printf("Rtsp Server Start:rtsp://%s:%hu/%s \n", config.ip.c_str(), config.port, config.suffix.c_str());

        std::lock_guard<std::mutex>  locker(mutex_);
        rtsp_server = rtsp_server;
        media_session_id_ = session_id;
    }
    else if (type == SCREEN_LIVE_RTSP_PUSHER) {
        auto rtsp_pusher = xop::RtspPusher::Create(event_loop_.get());
        xop::MediaSession *session = xop::MediaSession::CreateNew();
        session->AddSource(xop::channel_0, xop::H264Source::CreateNew());
        session->AddSource(xop::channel_1
                           , xop::AACSource::CreateNew(audio_capture_.GetSamplerate()
                           , audio_capture_.GetChannels(), false));
        rtsp_pusher->AddSession(session);
        if (rtsp_pusher->OpenUrl(config.rtsp_url, 1000) != 0) {
            rtsp_pusher = nullptr;
            printf("RTSP Pusher: Open url(%s) failed \n", config.rtsp_url.c_str());
            return false;
        }

        std::lock_guard<std::mutex> locker(mutex_);
        rtsp_pusher_ = rtsp_pusher;
        printf("RTSP Pusher start: Open stream to %s ... \n", config.rtsp_url.c_str());
    }
    else if (type == SCREEN_LIVE_RTMP_PUSHER) {
        auto rtmp_pusher = xop::RtmpPublisher::Create(event_loop_.get());

        xop::MediaInfo media_info;
        uint8_t extradata[1024] = { 0 };
        int extradata_size = 0;

        extradata_size = acc_encoder_.GetSpecificConfig(extradata, 1024);
        if (extradata_size <= 0) {
            printf("Get audio specific config failed");
            return false;
        }

        media_info.audio_specific_config_size = extradata_size;
        media_info.audio_specific_config.reset(new uint8_t[media_info.audio_specific_config_size]
                                               , std::default_delete<uint8_t[]>());
        memcpy(media_info.audio_specific_config.get(), extradata, extradata_size);

        extradata_size = h264_encoder_.GetSequenceParams(extradata, 1024);
        if (extradata_size <= 0) {
            printf("Get video specific config failed");
            return false;
        }

        xop::Nal sps = xop::H264Parser::findNal((uint8_t *)extradata, extradata_size);
        if (sps.first != nullptr && sps.second != nullptr && ((*sps.first & 0x1f) == 7)) {
            media_info.sps_size = sps.second - sps.first + 1;
            media_info.sps.reset(new uint8_t[media_info.sps_size], std::default_delete<uint8_t[]>());
            memcpy(media_info.sps.get(), sps.first, media_info.sps_size);

            xop::Nal pps = xop::H264Parser::findNal(sps.second, extradata_size - (sps.second - (uint8_t *)extradata));
            if (pps.first != nullptr && pps.second != nullptr && ((*pps.first & 0x1f) == 8)) {
                media_info.pps_size = pps.second - pps.first + 1;
                media_info.pps.reset(new uint8_t[media_info.pps_size], std::default_delete<uint8_t[]>());
                memcpy(media_info.pps.get(), pps.first, media_info.pps_size);
            }
        }
        rtmp_pusher->SetMediaInfo(media_info);

        std::string status;
        if (rtmp_pusher->OpenUrl(config.rtmp_url, 1000, status) < 0) {
            printf("RTMP Pusher: Open Url(%s) failed", config.rtmp_url.c_str());
            return false;
        }

        std::lock_guard<std::mutex> locker(mutex_);
        rtmp_pusher_ = rtmp_pusher;
        printf("RTMP Pusher start: Push Stream to %s ...", config.rtmp_url.c_str());
    }
    else {
        return false;
    }
    return true;

}

void ScreenLive::StopLive(int type)
{
    std::lock_guard<std::mutex> locker(mutex_);

    switch (type) {
    case SCREEN_LIVE_RTSP_SERVER:
        if (rtsp_server_ != nullptr) {
            rtsp_server_->Stop();
            rtsp_server_ = nullptr;
            rtsp_clients_.clear();
            printf("RTSP Server stop");
        }
    case SCREEN_LIVE_RTSP_PUSHER:
        if (rtsp_pusher_ != nullptr) {
            rtsp_pusher_->Close();
            rtsp_pusher_ = nullptr;
            printf("RTSP Pusher stop. \n");
        }
        break;
    case SCREEN_LIVE_RTMP_PUSHER:
        if (rtmp_pusher_ != nullptr) {
            rtmp_pusher_->Close();
            rtmp_pusher_ = nullptr;
            printf("RTMP Pusher stop. \n");
        }
        break;

    default:
        break;
    }
}

bool ScreenLive::IsConnected(int type)
{
    std::lock_guard<std::mutex> locker(mutex_);

    bool is_connected = false;
    switch (type) {
    case SCREEN_LIVE_RTSP_SERVER:
        if (rtsp_server_ != nullptr) {
            is_connected = rtsp_clients_.size() > 0;
        }
        break;

    case SCREEN_LIVE_RTSP_PUSHER:
        if (rtsp_pusher_ != nullptr) {
            is_connected = rtsp_pusher_->IsConnected();
        }
        break;

    case SCREEN_LIVE_RTMP_PUSHER:
        if (rtmp_pusher_ != nullptr) {
            is_connected = rtmp_pusher_->IsConnected();
        }
        break;
    default:
        break;
    }
    return is_connected;
}

int ScreenLive::StartCapture()
{
    std::vector<DX::Monitor> monitors = DX::GetMonitors();
    if (monitors.empty()) {
        printf("Monitor not found \n");
        return -1;
    }

    for (size_t index = 0; index < monitors.size(); index++) {
        printf("Monitor(%u) info: %dx%d \n", index,
               monitors[index].right - monitors[index].left,
               monitors[index].bottom - monitors[index].top);
    }

    int display_index = 0;

    if (!screen_capture_) {
        if (IsWindows8OrGreater()) {
            printf("DXGI screen capture start,monitor index: %d \n", display_index);
            screen_capture_ = new DXGIScreenCapture();
            if (!screen_capture_->Init(display_index)) {
                printf("DXGI Screen capture start failed, monitor index: %d \n", display_index);
                delete screen_capture_;

                printf("GDI Screen capture start, monitor index: %d \n", display_index);
                screen_capture_ = new GDIScreenCapture();
            }
        }
        else {
            printf("GDI Screen capture start, monitor index: %d \n", display_index);
            screen_capture_ = new GDIScreenCapture();
        }

        if (!screen_capture_->Init(display_index)) {
            printf("Screen capture start failed, monitor index: %d \n", display_index);
            delete screen_capture_;
            screen_capture_ = nullptr;
            return -1;
        }
    }

    if (!audio_capture_.Init()) {
        return -1;
    }

    is_capture_started_ = true;
    return 0;

}

int ScreenLive::StartEncoder(AVConfig& config)
{

}

int ScreenLive::StopEncoder()
{

}


bool ScreenLive::IsKeyFrame(const uint8_t* data, uint32_t size)
{

}

void ScreenLive::EncoderVideo()
{

}

void ScreenLive::EncoderAudio()
{

}

void ScreenLive::PushVideo(const uint8_t* data, uint32_t size, uint32_t timestamp)
{
    xop::AVFrame video_frame(size);
    video_frame.size = size - 4;
    video_frame.type = IsKeyFrame(data, size);
    video_frame.timestamp = timestamp;
    memcpy(video_frame.buffer.get(), data + 4, size - 4);

    if (size > 0) {

    }

}

void ScreenLive::PushAudio(const uint8_t* data, uint32_t size, uint32_t timestamp)
{
    xop::AVFrame audio_frame(size);
    audio_frame.timestamp = timestamp;
    audio_frame.type = xop::AUDIO_FRAME;
    audio_frame.size = size;
    memcpy(audio_frame.buffer.get(), data, size);

    if (size > 0) {
        std::lock_guard<std::mutex> locker(mutex_);

        if (rtsp_server_ != nullptr && this->rtsp_clients_.size() > 0)
            rtsp_server_->PushFrame(media_session_id_, xop::channel_1, audio_frame);

        if (rtsp_pusher_ && rtsp_pusher_->IsConnected())
            rtsp_pusher_->PushFrame(xop::channel_1, audio_frame);

        if (rtmp_pusher_ != nullptr && rtmp_pusher_->IsConnected())
            rtmp_pusher_->PushAudioFrame(audio_frame.buffer.get(), audio_frame.size);
    }
}



