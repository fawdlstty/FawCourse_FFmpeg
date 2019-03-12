// 编译前请在项目属性中关闭SDL检查
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>

// 该文件包含不能在当前代码页(936)中表示的字符。请将该文件保存为 Unicode 格式以防止数据丢失
#pragma warning (disable: 4819)
// warning C4996: 'xxxxxxxxxxx': 被声明为已否决
#pragma warning (disable: 4996)
#define __STDC_CONSTANT_MACROS

#ifdef __cplusplus
extern "C" {
#endif
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>
#ifdef main
#undef main
#endif
#ifdef __cplusplus
};
#endif

#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "avdevice.lib")
#pragma comment (lib, "avfilter.lib")
#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "swresample.lib")
#pragma comment (lib, "swscale.lib")
#pragma comment (lib, "SDL2.lib")

#define _SHOW_ERROR(x) printf ("%s failed\n", (x))



AVFormatContext *m_ifmt_ctx = nullptr;



// 视频格式转换
// 视频格式转换
AVFrame *_video_format_convet (AVFrame *_frame_src, AVPixelFormat _new_fmt, int _new_width = 0, int _new_height = 0) {
	AVFrame *_frame_dest = av_frame_alloc ();
	_frame_dest->width = (_new_width > 0 ? _new_width : _frame_src->width);
	_frame_dest->height = (_new_height > 0 ? _new_height : _frame_src->height);
	_frame_dest->format = _new_fmt;
	av_frame_get_buffer (_frame_dest, 0);
	// 初始化转码器
	SwsContext *_sws_ctx = sws_getContext (_frame_src->width, _frame_src->height, (AVPixelFormat) _frame_src->format, _frame_dest->width, _frame_dest->height, (AVPixelFormat) _frame_dest->format, SWS_BICUBIC, NULL, NULL, NULL);
	if (!_sws_ctx)
		printf ("sws_getContext failed\n");
	// 转码
	sws_scale (_sws_ctx, (const uint8_t* const*) _frame_src->data, _frame_src->linesize, 0, _frame_src->height, _frame_dest->data, _frame_dest->linesize);
	sws_freeContext (_sws_ctx);
	return _frame_dest;
}

// 程序入口点
int main (int argc, char* argv[]) {
	// 初始化ffmpeg
	av_register_all ();
	avformat_network_init ();
	avdevice_register_all ();
	avcodec_register_all ();
	avfilter_register_all ();
	SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

	//// SDL1：创建窗口与图片
	//SDL_Surface *_screen = SDL_SetVideoMode (640, 480, 0, SDL_SWSURFACE);
	//SDL_Overlay *_bmp = SDL_CreateYUVOverlay (640, 480, SDL_YV12_OVERLAY, _screen);
	//SDL_Rect _rect { 0, 0, 640, 480 };
	// SDL2：创建窗口与图片
	SDL_Window *_screen = SDL_CreateWindow ("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	SDL_Renderer* _sdlRenderer = SDL_CreateRenderer (_screen, -1, 0);
	SDL_Texture* _sdlTexture = SDL_CreateTexture (_sdlRenderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, 640, 480);
	SDL_Rect _rect { 0, 0, 640, 480 };

	// 查找输入流
	AVInputFormat *_ipt_fmt = av_find_input_format ("dshow");
	if (!_ipt_fmt) {
		_SHOW_ERROR ("av_find_input_format");
		return -1;
	}

	// 打开输入流
	const char *_in_url = "video=ICT Camera"; // 此处改为自己的摄像头名称
	int _ret = avformat_open_input (&m_ifmt_ctx, _in_url, _ipt_fmt, nullptr);
	if (_ret != 0) {
		_SHOW_ERROR ("avformat_open_input");
		return -1;
	}
	m_ifmt_ctx->flags |= AVFMT_FLAG_NOBUFFER;
	if ((_ret = avformat_find_stream_info (m_ifmt_ctx, nullptr)) < 0) {
		avformat_free_context (m_ifmt_ctx);
		m_ifmt_ctx = nullptr;
		_SHOW_ERROR ("avformat_find_stream_info");
		return -1;
	}

	// 遍历解码器
	for (unsigned int i = 0; i < m_ifmt_ctx->nb_streams; ++i) {
		AVCodec *_codec = avcodec_find_decoder (m_ifmt_ctx->streams [i]->codec->codec_id);
		if (!_codec) {
			avformat_free_context (m_ifmt_ctx);
			m_ifmt_ctx = nullptr;
			_SHOW_ERROR ("avcodec_find_decoder");
			return -1;
		}
		// 此处可调节音频设备默认采样率
		//if (m_ifmt_ctx->streams [i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		//	m_ifmt_ctx->streams [i]->codec->sample_rate = 48000;
		if (avcodec_open2 (m_ifmt_ctx->streams [i]->codec, _codec, nullptr) < 0) {
			avformat_free_context (m_ifmt_ctx);
			m_ifmt_ctx = nullptr;
			_SHOW_ERROR ("avcodec_open2");
			return -1;
		}
	}

	AVPacket *_pkt = av_packet_alloc ();
	AVFrame *_frame = av_frame_alloc ();

	while (true) {
		try {
			av_init_packet (_pkt);
			int _ret = av_read_frame (m_ifmt_ctx, _pkt);
			if (_ret < 0) {
				_SHOW_ERROR ("av_read_frame");
				break;
			}
			if (_pkt->stream_index < 0 || _pkt->stream_index > (int) m_ifmt_ctx->nb_streams) {
				std::this_thread::sleep_for (std::chrono::milliseconds (1));
				continue;
			}

			// 将原始帧包解码
			if ((_ret = avcodec_send_packet (m_ifmt_ctx->streams [_pkt->stream_index]->codec, _pkt)) != 0) {
				_SHOW_ERROR ("avcodec_send_packet");
				break;
			}
			av_frame_make_writable (_frame);
			_ret = avcodec_receive_frame (m_ifmt_ctx->streams [_pkt->stream_index]->codec, _frame);
			if (_ret == AVERROR (EAGAIN) || _ret == AVERROR_EOF) {
				std::this_thread::sleep_for (std::chrono::milliseconds (1));
				continue;
			} else if (_ret != 0) {
				_SHOW_ERROR ("avcodec_receive_frame");
				break;
			}

			AVFrame *_frame_yuv420p = _video_format_convet (_frame, AV_PIX_FMT_YUV420P, 640, 480);
			// SDL1：显示图片并延迟
			//SDL_LockYUVOverlay (_bmp);
			//memcpy (_bmp->pixels [0], _frame_yuv420p->data [0], _frame_yuv420p->width * _frame_yuv420p->height);
			//memcpy (_bmp->pixels [1], _frame_yuv420p->data [1], _frame_yuv420p->width * _frame_yuv420p->height / 4);
			//memcpy (_bmp->pixels [2], _frame_yuv420p->data [2], _frame_yuv420p->width * _frame_yuv420p->height / 4);
			//_frame_yuv420p->linesize [0] = _bmp->pitches [0];
			//_frame_yuv420p->linesize [1] = _bmp->pitches [2];
			//_frame_yuv420p->linesize [2] = _bmp->pitches [1];
			//SDL_UnlockYUVOverlay (_bmp);
			//SDL_DisplayYUVOverlay (_bmp, &_rect);

			// SDL2：显示图片
			int _sz = _frame_yuv420p->width * _frame_yuv420p->height;
			uint8_t *_buf = new uint8_t [_sz * 3 / 2];
			memcpy (_buf, _frame_yuv420p->data [0], _sz);
			memcpy (_buf + _sz * 5 / 4, _frame_yuv420p->data [1], _sz / 4);
			memcpy (_buf + _sz, _frame_yuv420p->data [2], _sz / 4);
			SDL_UpdateTexture (_sdlTexture, NULL, _buf, 640);
			SDL_RenderClear (_sdlRenderer);
			SDL_RenderCopy (_sdlRenderer, _sdlTexture, NULL, &_rect);
			SDL_RenderPresent (_sdlRenderer);
			delete _buf;

			av_frame_unref (_frame_yuv420p);
			av_frame_free (&_frame_yuv420p);
			av_packet_unref (_pkt);
			SDL_Delay (50);
		} catch (std::exception &e) {
			_SHOW_ERROR (e.what ());
		} catch (...) {
			_SHOW_ERROR ("catch error");
		}
		std::this_thread::sleep_for (std::chrono::milliseconds (1));
	}
	av_frame_free (&_frame);
	av_packet_free (&_pkt);

	// 释放
	SDL_Quit ();
	avformat_close_input (&m_ifmt_ctx);
	return 0;
}
