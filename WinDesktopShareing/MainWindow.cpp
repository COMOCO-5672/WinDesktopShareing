#include "MainWindow.h"
#include <mutex>

MainWindow::MainWindow()
{

}

MainWindow::~MainWindow()
{

}

bool MainWindow::Create()
{
    static std::once_flag init_flag;
    std::call_once(init_flag, [=] {
        SDL_assert(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    });

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    int window_flag = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
    window_ = SDL_CreateWindow("Screen Live", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED
                               , window_width_, window_height_, window_flag);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window_);
    SDL_SysWMinfo wm_info;
    SDL_VERSION(&wm_info.version);
    if (SDL_GetWindowWMInfo(window_, &wm_info)) {
        window_handle_ = wm_info.info.win.window;
    }

    if (!Init()) {
        Destory();
        return false;
    }

    if (window_handle_) {
        ImmAssociateContext((HWND)window_handle_, nullptr);
    }

    SDL_SetWindowMinimumSize(window_, kMinOverlayWidth, kMinOverlayHeight + 100);

    return (window_handle_ && window_);
}

void MainWindow::Destory()
{
    if (window_) {
        Clear();
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        window_handle_ = nullptr;
    }
}

bool MainWindow::IsWindow() const
{
    return window_ != nullptr;
}


void MainWindow::Resize()
{
    if (IsWindow()) {
        int width = 0, height = 0;
        SDL_GetWindowSize(window_, &width, &height);

        window_width_ = width;
        window_height_ = height;
        video_width_ = window_width_;
        video_height_ = window_height_ - kMinOverlayHeight;
        overlay_width_ = window_width_;
        overlay_height_ = kMinOverlayHeight;

        Clear();
        Init();
    }
}

bool MainWindow::Init()
{
    int driver_index = -1;
    int driver_count = SDL_GetNumRenderDrivers();
    int renderer_flags = SDL_RENDERER_SOFTWARE;

    renderer_name_ = "opengl";

    for (int i = 0; i < driver_count; i++) {
        SDL_RendererInfo info;
        if (SDL_GetRenderDriverInfo(i, &info) < 0) {
            continue;
        }

        if (strcmp(info.name, renderer_name_.c_str()) == 0) {
            driver_index = i;
            if (info.flags & SDL_RENDERER_ACCELERATED) {
                renderer_flags = SDL_RENDERER_ACCELERATED;
            }
        }
    }

    if (driver_index < 0) {
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, driver_index, renderer_flags);
    SDL_assert(renderer_ != nullptr);

    SDL_SetRenderDrawColor(renderer_, 114, 144, 154, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer_);
    SDL_RenderPresent(renderer_);

    overlay_ = new OverLay;

    SDL_RendererInfo renderer_info;
    SDL_GetRendererInfo(renderer_, &renderer_info);

    bool ret = false;
    if (strcmp(renderer_info.name, "direct3d") == 0) {
        device_ = SDL_RenderGetD3D9Device(renderer_);
        SDL_assert(device_ != nullptr);
        ret = overlay_->Init(window_, device_);
    }
    else if (strcmp(renderer_info.name, "opengl") == 0) {
        gl_context_ = SDL_GL_CreateContext(window_);
        SDL_assert(gl_context_ != nullptr);
        SDL_GL_MakeCurrent(window_, gl_context_);
        SDL_GL_SetSwapInterval(1);
        ret = overlay_->Init(window_, gl_context_);
    }

    if (!ret) {
        delete overlay_;
        overlay_ = nullptr;
    }

    overlay_->SetRect(0, 0 + video_height_, video_width_, kMinOverlayHeight);
    overlay_->RegisterObserver(this);
    return true;
}

void MainWindow::Clear()
{
    if (overlay_) {
        overlay_->Destroy();
        delete overlay_;
        overlay_ = nullptr;
    }

    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
        texture_format_ = SDL_PIXELFORMAT_UNKNOWN;
        texture_width_ = 0;
        texture_height_ = 0;
    }

    if (renderer_) {
        if (gl_context_) {
            SDL_GL_DeleteContext(gl_context_);
            gl_context_ = nullptr;
        }

        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
}

void MainWindow::Process(SDL_Event& event)
{
    if (IsWindow()) {
        if (overlay_)
            OverLay::Process(&event);
    }
}

void MainWindow::SetDebugInfo(std::string text)
{
    if (IsWindow()) {
        if (overlay_)
            debug_info_text_ = text;
    }
}

bool MainWindow::UpdateARGB(const uint8_t* data, uint32_t width, uint32_t height)
{
    if (!IsWindow()) {
        return false;
    }

    if (texture_format_ != SDL_PIXELFORMAT_ABGR8888
        || (texture_width_ != width) || (texture_height_ != height)) {
        if (texture_) {
            SDL_DestroyTexture(texture_);
            texture_ = nullptr;
        }
    }

    if (!texture_) {
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ABGR8888,
                                     SDL_TEXTUREACCESS_STREAMING, width, height);

        SDL_assert(texture_ != nullptr);
        texture_format_ = SDL_PIXELFORMAT_ABGR8888;
        texture_width_ = width;
        texture_height_ = height;
    }

    if (texture_) {
        char *pixels = nullptr;
        int pitch = 0;

        int ret = SDL_LockTexture(texture_, nullptr, (void **)&pixels, &pitch);
        if (ret < 0) {
            return false;
        }

        memcpy(pixels, data, texture_width_ * texture_height_ * 4);
        SDL_UnlockTexture(texture_);

        SDL_RenderClear(renderer_);

        SDL_Rect rect = { 0,0,video_width_,video_height_ };
        if (overlay_) {
            if (!debug_info_text_.empty()) {
                overlay_->SetDebugInfo(debug_info_text_);
            }
            overlay_->Render();
        }
        SDL_RenderPresent(renderer_);
        return true;
    }

    return false;
}

bool MainWindow::StartLive(int& event_type, std::vector<std::string>& encoder_settings, std::vector<std::string>& live_settings)
{
    AVConfig av_config;
    av_config.framerate = atoi(encoder_settings[1].c_str());
    av_config.bitrate_bps = atoi(encoder_settings[2].c_str()) * 1000U;
    av_config.codec = encoder_settings[0];

    if ((av_config.codec == "h264_nvenc" && !nvenc_info.is_supported())
        || (av_config.codec == "h264_qsv" && !QsvEncoder::IsSupported())) {
        av_config.codec = "x264";
    }

    if (av_config != avconfig_) {
        ScreenLive::Instance().StopLive(SCREEN_LIVE_RTSP_SERVER);
        ScreenLive::Instance().StopLive(SCREEN_LIVE_RTSP_PUSHER);
        ScreenLive::Instance().StopLive(SCREEN_LIVE_RTSP_PUSHER);

        overlay_->SetLiveState(EVENT_TYPE_RTSP_SERVER, false);
        overlay_->SetLiveState(EVENT_TYPE_RTSP_PUSHER, false);
        overlay_->SetLiveState(EVENT_TYPE_RTMP_PUSHER, false);
        ScreenLive::Instance().StopEncoder();
        if (ScreenLive::Instance().StartEncoder(av_config) < 0) {
            return false;
        }

        avconfig_ = av_config;
    }

    if (!ScreenLive::Instance().IsEncoderInited()) {
        return false;
    }

    LiveConfig live_config;
    bool ret = false;

    if (event_type == EVENT_TYPE_RTSP_SERVER) {
        live_config.ip = live_settings[0];
        live_config.port = atoi(live_settings[1].c_str());
        live_config.suffix = live_settings[2];
        ret = ScreenLive::Instance().StartLive(SCREEN_LIVE_RTSP_SERVER, live_config);
    }
    else if (event_type == EVENT_TYPE_RTSP_PUSHER) {
        live_config.rtsp_url = live_settings[0];
        ret = ScreenLive::Instance().StartLive(SCREEN_LIVE_RTSP_PUSHER, live_config);
    }
    else if (event_type == EVENT_TYPE_RTMP_PUSHER) {
        live_config.rtmp_url = live_settings[0];
        ret = ScreenLive::Instance().StartLive(SCREEN_LIVE_RTMP_PUSHER, live_config);
    }

    return ret;
}

void MainWindow::StopLive(int event_type)
{
    if (event_type == EVENT_TYPE_RTSP_SERVER) {
        ScreenLive::Instance().StopLive(SCREEN_LIVE_RTSP_SERVER);
    }
    else if (event_type == EVENT_TYPE_RTSP_PUSHER) {
        ScreenLive::Instance().StopLive(SCREEN_LIVE_RTSP_PUSHER);
    }
    else if (event_type == EVENT_TYPE_RTMP_PUSHER) {
        ScreenLive::Instance().StopLive(SCREEN_LIVE_RTMP_PUSHER);
    }
}


















