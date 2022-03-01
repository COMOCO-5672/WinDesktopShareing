#ifndef H265_ENCODER_H
#define H265_ENCODER_H

#include <string>
#include <vector>
#include "avcodec/"

class H265Encoder
{
public:
    H265Encoder& operator=(const H265Encoder&) = delete;
    H265Encoder(const H265Encoder&) = delete;
    H265Encoder();
    virtual ~H265Encoder();

    void SetCodec(std::string codec);

    bool Init(int framerate, int bitrate_kbps, int format, int width, int height);
    void Destroy();

    int Encode(uint8_t* in_buffer, uint32_t in_width, uint32_t in_height,
               uint32_t image_size, std::vector<uint8_t>& out_frame);

    int GetSequenceParams(uint8_t* out_buffer, int out_buffer_size);

private:
    bool IsKeyFrame(const uint8_t* data, uint32_t size);

    std::string codec_;
    ffmpeg::AVConfig encoder_config_;
	
};

#endif