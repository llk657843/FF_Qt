#pragma once
struct VideoEncoderParam
{
	VideoEncoderParam() {
		bitrate_ = 4000000;
		fps_ = 25;
		video_width_ = 1920;
		video_height_ = 1080;
	}
	int bitrate_;
	int fps_;
	int video_width_;
	int video_height_;
};