# 第五章 音视频格式处理

我们已经学会了FFmpeg基础的读写，下面我们将来学习如何编码解码。

## 音视频解码

音视频解码的位置在于，读到了一个AVPacket后，需要将其转为AVFrame，以便于接下来的处理。

主要由两个函数进行处理，avcodec_send_packet 与 avcodec_receive_frame，它们是一对一的关系，也就是说，send成功多少次，recv就会成功多少次。但这俩函数可能会出现缓存情况，也就是send之后，FFmpeg还没来得及解码，就调用了recv，这时候将会没有收到解码后的帧，这时候最佳处理方法是不管这一帧了，跳过recv之后的操作，从新开始下一轮的send。等send完毕之后，后续将没有读出的帧全给读出来。

包括后面的编码也同理。

```cpp
// 初始化帧
AVFrame *_frame = av_frame_alloc ();

// ...

// 将原始帧包解码
av_frame_make_writable (_frame);
if ((_ret = avcodec_send_packet (m_ifmt_ctx->streams [_pkt->stream_index]->codec, _pkt)) != 0) {
    printf ("avcodec_send_packet failed %d\n", _ret);
    break;
}
_ret = avcodec_receive_frame (m_fmt_ctx->streams [_pkt->stream_index]->codec, _frame);
if (_ret == AVERROR (EAGAIN) || _ret == AVERROR_EOF) {
    std::this_thread::sleep_for (std::chrono::milliseconds (1));
    continue;
} else if (_ret != 0) {
    printf ("avcodec_receive_frame failed %d\n", _ret);
    break;
}

// ...

// 释放帧
av_frame_free (&_frame);
```

## 音视频编码

音视频解码的位置在于，已经解码或处理完毕的AVFrame，需要将其转为AVPacket，以便于存文件或者推流。

```cpp
av_init_packet (_pkt);
int _ret = avcodec_send_frame (get_ctx (_stream_index), _frame);
if (_ret != 0) {
    printf ("avcodec_send_frame failed %d\n", _ret);
    return;
}
_ret = avcodec_receive_packet (get_ctx (_stream_index), m_pkt_send);
if (_ret != 0)
    break;
```

## 音视频格式转换

首先说说视频，这儿指的是 AVPixelFormat 枚举类型，需要转换的地方在于：

1. 当解码完成后的像素格式与需编码的像素格式不匹配的时候
2. 当需要将视频内容展示出来的时候
3. 需要调整大小的时候

第一个，举个最直观的例子。当读取的视频设备是摄像头，那么解码后的原始帧的AVFrame的像素格式为 AV_PIX_FMT_YUYV422，但如果想对其进行编码，转为H264编码，那么这个格式就不行，因为H264仅支持AV_PIX_FMT_YUV420P格式编码。也就是说，如果想把摄像头的数据转为H264，那么视频格式转换这一步是少不了的。

第二个，视频图像格式通常为YUV系列格式，但如果需要展示到屏幕上，通常需要RGB格式，所以这也是需要转换的。

第三个，像素格式转换兼具调整图片大小的功能。

为了方便调用，我此处提供一个转换函数，需注意的是此函数没有优化，每次转换都需要获取一个context；后续实际用于项目后，可以根据情况进行优化。

```cpp
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
```

接下来说说音频，这儿指的是 AVSampleFormat 枚举类型，需要转换的地方在于：

1. 当解码完成后的像素格式与需编码的像素格式不匹配的时候
2. 当需要单通道与双通道互转的时候
3. 需要调整采样率的时候

第一个，依然是例子：麦克风读到的原始 AVFrame 为 AV_SAMPLE_FMT_S16，当需要转为AAC格式，此时仅支持 AV_SAMPLE_FMT_FLTP 格式进行编码，所以此处需要转换采样格式。

第二个，单通道与双通道互转太正常了，比如有的麦克风是双通道但需要提取单通道；比如有的单通道音频需要转为双通道音频做进一步处理时，等等。

第三个，重采样也很常见，比如按照了48KHz频率采样后，需要转为44.1KHz然后做进一步处理，此处就需要做重采样了。

```cpp
// 音频格式转换
AVFrame *_audio_format_convet (AVFrame *_frame_src, AVSampleFormat _new_sample_fmt = AV_SAMPLE_FMT_NONE, int64_t _new_channel_layout = 0, int _new_sample_rate = 0) {
    _new_sample_fmt = (_new_sample_fmt != AV_SAMPLE_FMT_NONE ? _new_sample_fmt : (AVSampleFormat) _frame_src->format);
    _new_channel_layout = (_new_channel_layout > 0 ? _new_channel_layout : _frame_src->channel_layout);
    int _new_channels = av_get_channel_layout_nb_channels (_new_channel_layout), _ret = 0;
    _new_sample_rate = (_new_sample_rate > 0 ? _new_sample_rate : _frame_src->sample_rate);
    auto _param = std::make_tuple (_frame_src->channels, _frame_src->sample_rate, (AVSampleFormat) _frame_src->format, _new_channels, _new_sample_rate, (AVSampleFormat) _new_sample_fmt);
    SwrContext *_swr_ctx = swr_alloc ();
    av_opt_set_int (_swr_ctx, "in_channel_count", _frame_src->channels, 0);
    av_opt_set_int (_swr_ctx, "out_channel_count", _new_channels, 0);
    av_opt_set_int (_swr_ctx, "in_sample_rate", _frame_src->sample_rate, 0);
    av_opt_set_int (_swr_ctx, "out_sample_rate", _new_sample_rate, 0);
    av_opt_set_int (_swr_ctx, "in_sample_fmt", _frame_src->format, 0);
    av_opt_set_int (_swr_ctx, "out_sample_fmt", _new_sample_fmt, 0);
    if ((_ret = swr_init (_swr_ctx)) != 0) {
        printf ("swr_init failed %d\n", _ret);
        swr_free (&_swr_ctx);
    }
    AVFrame *_frame_dest = av_frame_alloc ();
    _frame_dest->nb_samples = (_frame_src->nb_samples + 100) * _new_sample_rate / _frame_src->sample_rate;
    _frame_dest->channels = _new_channels;
    _frame_dest->channel_layout = _new_channel_layout;
    _frame_dest->format = _new_sample_fmt;
    _frame_dest->sample_rate = _new_sample_rate;
    av_frame_get_buffer (_frame_dest, 0);
    _ret = swr_convert (_swr_ctx, _frame_dest->data, _frame_dest->nb_samples, (const uint8_t**) _frame_src->data, _frame_src->nb_samples);
    if (_ret <= 0) {
        printf ("swr_convert failed %d\n", _ret);
    } else {
        _frame_dest->nb_samples = _ret;
    }
    swr_free (&_swr_ctx);
    return _frame_dest;
}
```

## 程序结构

下面我给出一个实战的将摄像头和麦克风数据读取，然后存为文件的伪代码的示例。当然此处是有问题的，单线程肯定不合适，不过，至少实现了。

```cpp
#include <头文件>

int main (int argc, char* argv[]) {
    // 初始化
    // 打开摄像头
    // 打开麦克风
    // 打开输出流
    while (_run) {
        // 图像处理
        // 从摄像头输入流中读一个AVPacket
        // 将AVPacket解码为AVFrame
        // 根据实际需求考虑是否需要转换AVFrame的像素格式
        // 将AVFrame编码为AVPacket
        // send一帧
        //
        // 音频处理
        // 从麦克风输入流中读一个AVPacket
        // 将AVPacket解码为AVFrame
        // 根据实际需求考虑是否需要转换AVFrame的采样格式
        // 将AVFrame编码为AVPacket
        // send一帧
        //
        // 控制速度
    }
    // 关闭输出流
    // 关闭麦克风输入流
    // 关闭摄像头输入流
    return 0;
}
```

接下来是多线程架构，基本上可以用于项目开发中了：

```cpp
#include <头文件>

int main (int argc, char* argv[]) {
    // 初始化
    // 打开摄像头
    // 打开麦克风
    // 打开输出流
    new_thread {
        while (_run) {
            // 图像处理
            // 从摄像头输入流中读一个AVPacket
            // 将AVPacket解码为AVFrame
            // 根据实际需求考虑是否需要转换AVFrame的像素格式
            // 将AVFrame编码为AVPacket
            // send一帧
            // 控制速度
        }
    }
    new_thread {
        while (_run) {
            // 音频处理
            // 从麦克风输入流中读一个AVPacket
            // 将AVPacket解码为AVFrame
            // 根据实际需求考虑是否需要转换AVFrame的采样格式
            // 将AVFrame编码为AVPacket
            // send一帧
            // 控制速度
        }
    }
    while (_run) {
        // wait...
    }
    // 关闭输出流
    // 关闭麦克风输入流
    // 关闭摄像头输入流
    return 0;
}
```

[返回首页](../README.md) | [上一章 Hello FFmpeg](./04_hello_ffmpeg.md) | [下一章 FFmpeg与其他库的交互](./06_interaction.md)

## 许可

[![test](https://i.creativecommons.org/l/by-nc-nd/4.0/80x15.png)](http://creativecommons.org/licenses/by-nc-nd/4.0/)

本教程采用[知识共享署名-非商业性使用-禁止演绎 4.0 国际许可协议](http://creativecommons.org/licenses/by-nc-nd/4.0/)许可。
