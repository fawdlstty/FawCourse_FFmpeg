// 编译前请在项目属性中关闭SDL检查
#include <SDKDDKVer.h>
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <thread>
#include <chrono>
#include <mutex>

#include <gdiplus.h>
#pragma comment (lib, "gdiplus.lib")

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

#define _SHOW_ERROR(x) MessageBox (NULL, (x), _T ("提示"), MB_ICONHAND);



AVFormatContext *m_ifmt_ctx = nullptr;
bool m_run = true;
std::thread m_thread;
std::mutex m_mutex;
AVFrame *m_frame_rgb32 = nullptr;



// 视频格式转换
// 视频格式转换
AVFrame *_video_format_convert (AVFrame *_frame_src, AVPixelFormat _new_fmt, int _new_width = 0, int _new_height = 0) {
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

// 消息循环
LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_PAINT:
			{
				// 数据移动至GDI+
				m_mutex.lock ();
				if (m_frame_rgb32 == nullptr) {
					m_mutex.unlock ();
					return 0;
				}
				Gdiplus::Bitmap _bmp (m_frame_rgb32->width, m_frame_rgb32->height, PixelFormat32bppARGB);
				Gdiplus::BitmapData _data;
				_bmp.LockBits (&Gdiplus::Rect (0, 0, _bmp.GetWidth (), _bmp.GetHeight ()), Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &_data);
				memcpy (_data.Scan0, m_frame_rgb32->data [0], m_frame_rgb32->width * m_frame_rgb32->height * 4);
				_bmp.UnlockBits (&_data);
				m_mutex.unlock ();

				// 绘制
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint (hWnd, &ps);
				Gdiplus::Graphics g (hdc);
				g.DrawImage (&_bmp, Gdiplus::Rect (0, 0, 640, 480), 0, 0, 640, 480, Gdiplus::UnitPixel);
				EndPaint (hWnd, &ps);
			}
			break;
		case WM_DESTROY:
			PostQuitMessage (0);
			break;
		default:
			return DefWindowProc (hWnd, message, wParam, lParam);
	}
	return 0;
}

// 程序入口点
int APIENTRY _tWinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	// 注册窗口类
	WNDCLASSEX wcex { sizeof (WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor (nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszClassName = _T ("MyWindowClass");
	RegisterClassEx (&wcex);

	// 创建窗口
	HWND hWnd = CreateWindow (_T ("MyWindowClass"), _T ("My Window"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, hInstance, nullptr);
	if (!hWnd) {
		UnregisterClass (_T ("MyWindowClass"), hInstance);
		return -1;
	}
	ShowWindow (hWnd, nCmdShow);
	UpdateWindow (hWnd);

	// 初始化ffmpeg
	av_register_all ();
	avformat_network_init ();
	avdevice_register_all ();
	avcodec_register_all ();
	avfilter_register_all ();

	// 查找输入流
	AVInputFormat *_ipt_fmt = av_find_input_format ("dshow");
	if (!_ipt_fmt) {
		_SHOW_ERROR (_T ("av_find_input_format"));
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

	// 初始化GDI+
	ULONG_PTR _token = 0;
	Gdiplus::GdiplusStartupInput _input;
	Gdiplus::GdiplusStartupOutput _output;
	Gdiplus::GdiplusStartup (&_token, &_input, &_output);

	// 创建读摄像头线程
	m_thread = std::thread ([hWnd] {
		AVPacket *_pkt = av_packet_alloc ();
		AVFrame *_frame = av_frame_alloc ();
		while (m_run) {
			try {
				// 读一个原始帧包
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

				// 将图片放置到缓冲区
				if (_pkt->stream_index == 0) {
					std::unique_lock<std::mutex> ul (m_mutex);
					if (m_frame_rgb32) {
						av_frame_unref (m_frame_rgb32);
						av_frame_free (&m_frame_rgb32);
					}
					m_frame_rgb32 = _video_format_convert (_frame, AV_PIX_FMT_RGB32, 640, 480);
					RECT rect { 0, 0, 640, 480 };
					InvalidateRect (hWnd, &rect, FALSE);
				}
				av_packet_unref (_pkt);
			} catch (std::exception &e) {
				_SHOW_ERROR (e.what ());
			} catch (...) {
				_SHOW_ERROR (_T ("catch error"));
			}
			std::this_thread::sleep_for (std::chrono::milliseconds (1));
		}
		av_frame_free (&_frame);
		av_packet_free (&_pkt);
	});

	// 进入消息循环
	MSG msg { 0 };
	while (GetMessage (&msg, nullptr, 0, 0)) {
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}

	// 释放
	m_run = false;
	if (m_thread.joinable ())
		m_thread.join ();
	avformat_close_input (&m_ifmt_ctx);
	UnregisterClass (_T ("MyWindowClass"), hInstance);
	Gdiplus::GdiplusShutdown (_token);
	return (int) msg.wParam;
}
