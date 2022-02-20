#include "ScreenLive.h"
#include "MainWindow.h"

static const int SDL_USEREVENT_PAINT = 0x01;

static void OnPaint(void *param)
{
    MainWindow *window = reinterpret_cast<MainWindow*>(param);

    if (window) {
        std::vector<uint8_t> bgra_image;
        uint32_t width = 0;
        uint32_t height = 0;
        if (ScreenLive::Instance().GetScreenImage(bgra_image, width, height)) {
            std::string status_info = ScreenLive::Instance().GetStatusInfo();
            window->SetDebugInfo(status_info);
            window->UpdateARGB(&bgra_image[0], width, height);
        }
    }
}

static uint32_t TimerCallback(uint32_t interval, void *param)
{
    SDL_Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.user.type = SDL_USEREVENT;
    evt.user.timestamp = 0;
    evt.user.code = SDL_USEREVENT_PAINT;
    evt.user.data1 = param;
    evt.user.data2 = nullptr;
    SDL_PushEvent(&evt);

    return 100;
}

int main(int argc, char **argv)
{
    MainWindow window;
    SDL_TimerID timer_id = 0;

    if (window.Create()) {
        if (ScreenLive::Instance().StartCapture() >= 0) {
            timer_id = SDL_AddTimer(1000, TimerCallback, &window);
        }
        else {
            return -1;
        }
    }

    bool done = false;
    while (!done && window.IsWindow()) {
        SDL_Event event;
        if (SDL_WaitEvent(&event)) {
            window.Process(event);

            switch (event.type) {
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    window.Resize();
                }
                break;
            case SDL_USEREVENT:
                if (event.user.code == SDL_USEREVENT_PAINT) {
                    OnPaint(event.user.data1);
                }
                break;
            case SDL_QUIT:
                done = true;
                break;
            default:
                break;;
            }
        }
    }

    if (timer_id) {
        SDL_RemoveTimer(timer_id);
    }

    window.Destory();
    ScreenLive::Instance().Destory();
    return 0;
}