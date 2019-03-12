# 第六章 FFmpeg与其他库的交互

视频软件不是仅靠一个FFmpeg就能完成。FFmpeg只包括流的读写及编码解码，其他操作都得靠其他库来实现。比如播放视频需要界面库；播放声音需要播放声音的库或系统API等。所以这一块也是重中之重。下面我介绍几种常用FFmpeg与其他库交互方式。

## AVFrame与Qt图片互转

AVFrame转QImage比较简单，不过这儿有一个坑在于，直接指定数据的方式上面，一旦数据失效那么图片也会失效。此处的比较友好的做法是再构造一次

```cpp
// AVFrame 转 QImage
// 首先确保图像帧格式为 AV_PIX_FMT_RGB32，如果不是，那么转一次
AVFrame *_frame_rgb32 = _video_format_convet (_frame_xxx, AV_PIX_FMT_RGB32);
// 此处 _img_tmp 即可用，与 _frame_rgb32 共用数据区域
QImage _img_tmp (_frame_rgb32->data [0], _frame_rgb32->width, _frame_rgb32->height, QImage::Format_RGB32);
//但如果想让 _frame_rgb32 释放后继续使用，那么需要再构造一次
QImage _img { 640, 480, QImage::Format_ARGB32 };
QPainter _p (&_img);
_p.drawImage (_img.rect (), _img_tmp, _img_tmp.rect ());

// QImage 转 AVFrame
AVFrame *_frame_rgb32 = av_frame_alloc ();
_frame_rgb32->width = img.width ();
_frame_rgb32->height = img.height ();
_frame_rgb32->format = AV_PIX_FMT_RGB32;
av_frame_get_buffer (_frame_rgb32, 0);
memcpy (_frame_rgb32->data [0], img.bits (), _frame_rgb32->width * _frame_rgb32->height * 4);
```

## AVFrame与GDI+图片互转

AVFrame转Gdiplus::Bitmap也是比较容易的，不过还是需要注意GDI+初始化不要忘了

```cpp
// AVFrame 转 Gdiplus::Bitmap
// 此时 _frame_rgb32 为有效 AVFrame
Gdiplus::Bitmap _bmp (_frame_rgb32->width, _frame_rgb32->height, PixelFormat32bppARGB);
Gdiplus::BitmapData _data;
_bmp.LockBits (&Gdiplus::Rect (0, 0, _bmp.GetWidth (), _bmp.GetHeight ()), Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &_data);
memcpy (_data.Scan0, _frame_rgb32->data [0], _frame_rgb32->width * _frame_rgb32->height * 4);
_bmp.UnlockBits (&_data);

// Gdiplus::Bitmap 转 AVFrame
// 此时 _bmp 为有效 Gdiplus::Bitmap
AVFrame *_frame_rgb32 = av_frame_alloc ();
_frame_rgb32->width = _bmp.GetWidth ();
_frame_rgb32->height = _bmp.GetHeight ();
_frame_rgb32->format = AV_PIX_FMT_RGB32;
av_frame_get_buffer (_frame_rgb32, 0);
Gdiplus::BitmapData _data;
_bmp.LockBits (&Gdiplus::Rect (0, 0, _bmp.GetWidth (), _bmp.GetHeight ()), Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &_data);
memcpy (_frame_rgb32->data [0], _data.Scan0, _frame_rgb32->width * _frame_rgb32->height * 4);
_bmp.UnlockBits (&_data);
```

## SDL1播放视频

```cpp
// 首先是SDL初始化代码
SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

// 然后是创建SDL2窗口及SDL2图片
SDL_Surface *_screen = SDL_SetVideoMode (_frame_yuv420p->width, _frame_yuv420p->height, 0, SDL_SWSURFACE);
SDL_Overlay *_bmp = SDL_CreateYUVOverlay (_frame_yuv420p->width, _frame_yuv420p->height, SDL_YV12_OVERLAY, _screen);
SDL_Rect _rect { 0, 0, _frame_yuv420p->width, _frame_yuv420p->height };

// 然后是循环播放内部，将AVFrame结构体数据移动到SDL图片上
SDL_LockYUVOverlay (_bmp);
memcpy (_bmp->pixels [0], _frame_yuv420p->data [0], _frame_yuv420p->width * _frame_yuv420p->height);
memcpy (_bmp->pixels [1], _frame_yuv420p->data [1], _frame_yuv420p->width * _frame_yuv420p->height / 4);
memcpy (_bmp->pixels [2], _frame_yuv420p->data [2], _frame_yuv420p->width * _frame_yuv420p->height / 4);
_frame_yuv420p->linesize [0] = _bmp->pitches [0];
_frame_yuv420p->linesize [1] = _bmp->pitches [2];
_frame_yuv420p->linesize [2] = _bmp->pitches [1];
SDL_UnlockYUVOverlay (_bmp);
SDL_DisplayYUVOverlay (_bmp, &_rect);
// 暂停
SDL_Delay (50);

// 展示完毕，退出
SDL_Quit ();
```

## SDL2播放视频

```cpp
// 首先是SDL初始化代码
SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

// 然后是创建SDL2窗口及SDL2图片
SDL_Window *_screen = SDL_CreateWindow ("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
SDL_Renderer* _sdlRenderer = SDL_CreateRenderer (_screen, -1, 0);
SDL_Texture* _sdlTexture = SDL_CreateTexture (_sdlRenderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, 640, 480);
SDL_Rect _rect { 0, 0, 640, 480 };

// 然后是循环播放内部，将AVFrame结构体数据移动到SDL2图片上
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
// 暂停
SDL_Delay (50);

// 展示完毕，退出
SDL_Quit ();
```

## SDL2播放音频

SDL2播放音频有一个问题在于，仅支持单例。也就是不能创建两个对象一起播放。

```cpp
// 全局变量，用于控制播音进度，通常实现写在类里面作为类成员变量
volatile Uint8 *_audio_pos = nullptr;
volatile Uint32 _audio_len = 0;

// 回调函数。实际播音通过这个函数来实现
void _fill_audio (void *udata, Uint8 *stream, int len) {
    // 如果控制进度写到类/结构体成员，那么udata就传递指针
    SDL_memset (stream, 0, len);
    if (_audio_len == 0)
        return;
    len = (len > (int) _audio_len ? _audio_len : len);
    //SDL_MixAudio (stream, (const Uint8*) _audio_pos, len, SDL_MIX_MAXVOLUME);
    SDL_MixAudioFormat (stream, (const Uint8*) _audio_pos, AUDIO_S16, len, SDL_MIX_MAXVOLUME);
    _audio_pos += len;
    _audio_len -= len;
}

// 初始化
SDL_AudioSpec _spec;
_spec.freq = _frame_s16->sample_rate;
_spec.format = AUDIO_S16SYS;
_spec.channels = _frame_s16->channels;
_spec.silence = 0;
_spec.samples = 1024;
_spec.callback = &AudioPlay2::_fill_audio;
_spec.userdata = nullptr; // 或者 this，根据需求决定是否传递
if (int _ret = SDL_OpenAudio (&_spec, NULL)) {
    printf ("SDL_OpenAudio failed\n");
    return;
}
SDL_PauseAudio (0);

// 开始播放过程，每读一帧就调用一遍
_audio_pos = _frame_s16->data [0];
_audio_len = _frame_s16->nb_samples * _frame_s16->channels * 2;
while (_audio_len > 0)
    SDL_Delay (1);

// 释放
SDL_CloseAudio ();
```

## wave函数播放音频

wave系列函数仅支持Windows平台，范围从WinXP到Win10，比较古老，但相对于SDL2来说可以少引用一个库。另外这两者的实现特别像，几乎可以不用修改架构实现与SDL2的互相迁移。

```cpp
// 全局变量，用于控制播放进度
volatile LPWAVEHDR _pwh = nullptr;

// 回调函数具有Win32特色，需要指定调用协定
void CALLBACK _wave_out_proc (HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    // 此处 dwInstance 为用户指定的指针
    if (WOM_DONE == uMsg) {
        while (_pwh) {
            std::this_thread::sleep_for (std::chrono::milliseconds (1));
        }
        _pwh = (LPWAVEHDR) dwParam1;
    }
}

// 初始化
WAVEFORMATEX _wfex;
_wfex.wFormatTag = WAVE_FORMAT_PCM;
_wfex.nChannels = (WORD) _frame_s16->channels;
_wfex.nSamplesPerSec = (DWORD) _frame_s16->sample_rate;
_wfex.wBitsPerSample = 16;
_wfex.cbSize = sizeof (_wfex);
_wfex.nBlockAlign = _wfex.wBitsPerSample * _wfex.nChannels / 8;
_wfex.nAvgBytesPerSec = _wfex.nBlockAlign;
if (::waveOutOpen (nullptr, 0, &_wfex, 0, 0, WAVE_FORMAT_QUERY) != 0) {
    printf ("waveOutOpen failed\n");
    return;
}if (::waveOutOpen (&_hwo, WAVE_MAPPER, &_wfex, (DWORD_PTR) _wave_out_proc, (DWORD_PTR) this, CALLBACK_FUNCTION) != 0) {
    printf ("waveOutOpen failed\n");
    return;
}

// 开始播放过程，每读一帧就调用一遍
LPWAVEHDR _pwh = new WAVEHDR;
if (!_pwh)
    return false;
memset (_pwh, 0, sizeof (WAVEHDR));
_pwh->dwLoops = 1;
_pwh->dwBufferLength = (DWORD) _frame_s16->nb_samples * _frame_s16->channels * 2;
_pwh->lpData = new char [_pwh->dwBufferLength];
if (!_pwh->lpData) {
    delete _pwh;
    return;
}
memcpy (_pwh->lpData, _frame_s16->data[0], _pwh->dwBufferLength);
if (::waveOutPrepareHeader (_hwo, _pwh, sizeof (WAVEHDR)) != 0) {
    delete [] _pwh->lpData;
    delete _pwh;
    return;
}
if (::waveOutWrite (_hwo, _pwh, sizeof (WAVEHDR)) != 0) {
    delete [] _pwh->lpData;
    delete _pwh;
    return;
}

// 释放
if (_hwo) {
    ::waveOutReset (_hwo);
    ::waveOutClose (_hwo);
    _hwo = NULL;
}
```

## 枚举dshow设备

这个用到了dshow函数，操作稍微有点麻烦，获取一个设备需要一大堆代码。建议不要深究，需要的时候直接复制就好啦。

此处不用FFmpeg的原因是，截至目前FFmpeg暂不支持以编程的方式获取列表，如果通过读控制台的方式还涉及到解析等，说白了还是FFmpeg的锅。

地址：[https://trac.ffmpeg.org/wiki/DirectShow#Howtoprogrammaticallyenumeratedevices](https://trac.ffmpeg.org/wiki/DirectShow#Howtoprogrammaticallyenumeratedevices)

```cpp
// dshow头文件
#include <string>
#include <vector>
#include <dshow.h>
#include <dvdmedia.h>
#pragma comment (lib, "Strmiids.lib")
#pragma comment (lib, "Winmm.lib")

// 注意调用枚举前需要调用COM+初始化
::CoInitializeEx (NULL, COINIT_APARTMENTTHREADED);

// 枚举所有dshow视频设备
std::vector<std::wstring> _video_names;
ICreateDevEnum* pSysDevEnum = nullptr;
if (SUCCEEDED (CoCreateInstance (CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**) &pSysDevEnum))) {
    IEnumMoniker* pEnumCat = nullptr;
    if (SUCCEEDED (pSysDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pEnumCat, 0))) {
        IMoniker* pMoniker = nullptr;
        ULONG cFetched = 0;
        while (SUCCEEDED (pEnumCat->Next (1, &pMoniker, &cFetched)) && cFetched) {
            IPropertyBag* pPropBag = nullptr;
            if (SUCCEEDED (pMoniker->BindToStorage (nullptr, nullptr, IID_IPropertyBag, (void**) &pPropBag))) {
                VARIANT varName;
                VariantInit (&varName);
                if (SUCCEEDED (pPropBag->Read (L"FriendlyName", &varName, 0))) {
                    _video_names.push_back (varName.bstrVal);
                }
                VariantClear (&varName);
                if (pPropBag)
                    pPropBag->Release ();
            }
            if (pPropBag)
                pPropBag->Release ();
        }
        if (pMoniker)
            pMoniker->Release ();
    }
    if (pEnumCat)
        pEnumCat->Release ();
}
if (pSysDevEnum)
    pSysDevEnum->Release ();

// 枚举所有dshow音频设备
std::vector<std::wstring> _audio_names;
ICreateDevEnum* pSysDevEnum = NULL;
if (SUCCEEDED (CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**) &pSysDevEnum))) {
    IEnumMoniker* pEnumCat = nullptr;
    if (SUCCEEDED (pSysDevEnum->CreateClassEnumerator (CLSID_AudioInputDeviceCategory, &pEnumCat, 0))) {
        IMoniker* pMoniker = nullptr;
        ULONG cFetched;
        while (SUCCEEDED (pEnumCat->Next (1, &pMoniker, &cFetched)) && cFetched) {
            IPropertyBag* pPropBag = nullptr;
            if (SUCCEEDED (pMoniker->BindToStorage (nullptr, nullptr, IID_IPropertyBag, (void**) &pPropBag))) {
                VARIANT varName;
                VariantInit (&varName);
                if (SUCCEEDED (pPropBag->Read (L"FriendlyName", &varName, 0)))
                    _audio_names.push_back (varName.bstrVal);
                VariantClear (&varName);
            }
            if (pPropBag)
                pPropBag->Release ();
            pMoniker->Release ();
            pMoniker = nullptr;
        }
        if (pMoniker)
            pMoniker->Release ();
    }
    if (pEnumCat)
        pEnumCat->Release ();
}
if (pSysDevEnum)
    pSysDevEnum->Release ();
```

## 程序结构

对于播放器来说，只需要解码然后展示就行了

```cpp
#include <头文件>

int main (int argc, char* argv[]) {
    // 初始化
    // 打开输入流
    new_thread {
        while (_run) {
            // 图像处理
            // 从摄像头输入流中读一个AVPacket
            // 将AVPacket解码为AVFrame
            // 根据实际需求考虑是否需要转换AVFrame的像素格式
            // 展示图片（SDL2或者其他界面库）
        }
    }
    new_thread {
        while (_run) {
            // 音频处理
            // 从麦克风输入流中读一个AVPacket
            // 将AVPacket解码为AVFrame
            // 根据实际需求考虑是否需要转换AVFrame的采样格式
            // 播放声音（SDL2或其他音频库）
        }
    }
    while (_run) {
        // wait...
    }
    // 关闭输入流
    return 0;
}
```

对于一个视频监控软件来说，由于需要一边读摄像头一边展示，还要一边存文件，但不用读声音了，所以架构就是把这两者合并起来

```cpp
#include <头文件>

int main (int argc, char* argv[]) {
    // 初始化
    // 打开摄像头
    // 打开输出流
    new_thread {
        while (_run) {
            // 图像处理
            // 从摄像头输入流中读一个AVPacket
            // 将AVPacket解码为AVFrame
            // 根据实际需求考虑是否需要转换AVFrame的像素格式
            // 展示图片（SDL2或者其他界面库）
            // 将AVFrame编码为AVPacket
            // send一帧
            // 控制速度
        }
    }
    while (_run) {
        // wait...
    }
    // 关闭输出流
    // 关闭摄像头输入流
    return 0;
}
```

[返回首页](../README.md) | [上一章 音视频格式处理](./05_format_process.md) | [下一章 Filter 滤镜](./07_filter.md)

## 许可

[![test](https://i.creativecommons.org/l/by-nc-nd/4.0/80x15.png)](http://creativecommons.org/licenses/by-nc-nd/4.0/)

本教程采用[知识共享署名-非商业性使用-禁止演绎 4.0 国际许可协议](http://creativecommons.org/licenses/by-nc-nd/4.0/)许可。
