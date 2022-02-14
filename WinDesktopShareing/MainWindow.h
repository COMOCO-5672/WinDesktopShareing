#ifndef SCREEN_LIVE_MAINWINDOW_H
#define SCREEN_LIVE_MAINWINDOW_H

#include "SDL.h"
#include "SDL_syswm.h"
#include "OverLay.h"
#include "ScreenLive.h"
#include <string>
#include <Windows.h>

class MainWindow :public OverLayCallBack {
public:
    MainWindow();
    virtual ~MainWindow();

    bool create();
    void Destory();
    bool IsWindow() const;
    void Resize();

    void Process(SDL_Event &event);
    void SetDebugInfo(std::string text);
    bool UpdateARGB(const uint8_t *data, uint32_t width, uint32_t height);

private:
    bool Init();
    void Clear();

    virtual bool StartLive(int& event_type
                           , std::vector<std::string>& encoder_settings
                           , std::vector<std::string>& live_settings) override;

    virtual void StopLive(int event_type) override;

    SDL_Window *window_{ nullptr };
    HWND        window_handle_{ nullptr };

    OverLay    *overlay_{ nullptr };
    std::string debug_info_text_;

    AVConfig avconfig_;
    std::string render_name_;
    SDL_Renderer *rederer_{ nullptr };


};
#endif
