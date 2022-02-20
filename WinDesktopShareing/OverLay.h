#ifndef OVER_LAY_H
#define OVER_LAY_H

#include "SDL.h"
#include <string>
#include <vector>

enum OverLayEventType {
	EVENT_TYPE_RTSP_SERVER = 0x001,
	EVENT_TYPE_RTSP_PUSHER = 0x002,
	EVENT_TYPE_RTMP_PUSHER = 0x003,
};

class OverLayCallBack {
public:
	virtual bool StartLive(int& event_type
		, std::vector<std::string>& encoder_settings
		, std::vector<std::string>& live_settings) = 0;

	virtual void StopLive(int event_type) = 0;

	virtual ~OverLayCallBack();
};

class OverLay {
public:
	OverLay();
	virtual ~OverLay();

	void RegisterObserver(OverLayCallBack* call_back);

	bool Init(SDL_Window* window, IDirect3DDevice9* device9);
	bool Init(SDL_Window* window, SDL_GLContext gl_context);

	void SetRect(int x, int y, int w, int h);
	void Destory();
	bool Render();

	static void Process(SDL_Event* event);

	void SetLiveState(int event_type, bool state);
	void SetDebugInfo(std::string text);

private:
	void Init();
	bool Copy();
	bool Begin();
	bool End();
	void NotifyEvent();

	SDL_Window* window{ nullptr };

};
#endif
