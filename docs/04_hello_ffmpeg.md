# 第四章 Hello FFmpeg

## 头文件

头文件是我最不想管的地方，多引入了一些没啥，少引入了一些直接报错，所以通常我就直接将这一堆代码直接复制过来，多了就多了，没问题，如果少了那么再加。

```cpp
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
//#include <SDL2/SDL.h>
//#ifdef main
//#undef main
//#endif
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
//#pragma comment (lib, "SDL2.lib")
```

## 初始化

按照我的说法，初始化这一块也不用管了，写了好像也没啥意义，不写反而运行报错。所以这一块也不用太深究。

头文件与初始化中注释的部分包括了SDL2部分，如果需要用到SDL2那么直接解注释即可。

```cpp
av_register_all ();
avformat_network_init ();
avdevice_register_all ();
avcodec_register_all ();
avfilter_register_all ();
//SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
```

## 打开摄像头/麦克风

现在开始真正进入实际操作了，首先是打开设备。打开设备前首先需要指定设备类型。比如windows上通常以vfw、dshow打开摄像头，gdigrab打开屏幕录制，nullptr打开文件或拉流（比如看网红直播）。此处我们打开摄像头，就以dshow为示例：

```cpp
// 查找input format
const char *_fmt_name = "dshow"; // nullptr or "gdigrab" etc.
AVInputFormat *_ipt_fmt = nullptr;
if (_fmt_name && _fmt_name [0]) {
    if (!(_ipt_fmt = av_find_input_format (_fmt_name))) {
        printf ("av_find_input_format failed\n");
        return;
    }
}
```

然后开始打开摄像头，摄像头名称可以通过以下命令行获取：

`ffmpeg -list_devices true -f dshow -i dummy`

这儿需要注意的是由于列表展示的是utf8编码，所以很可能命令行里面的汉字乱码；其次调用FFmpeg传入摄像头或麦克风名称为utf8编码，所以在传入的地方记得转码。

如果需要在程序中获取摄像头名称，那么最佳方式是通过dshow函数来获取摄像头列表，然后FFmpeg中使用dshow来打开摄像头。关于获取dshow摄像头列表我将在后面讲。

注意打开摄像头设备的前缀是`video=`，打开麦克风设备前缀是`audio=`，打开拉流地址就可以直接写url了：`rtmp://xxxx...`

```cpp
// 打开摄像头设备
const char *_in_url = "video=My Camera Name";
AVFormatContext *m_ifmt_ctx = nullptr;
int _ret = avformat_open_input (&m_ifmt_ctx, _in_url, _ipt_fmt, nullptr);
if (_ret != 0) {
    printf ("avformat_open_input failed\n");
    return;
}
m_ifmt_ctx->flags |= AVFMT_FLAG_NOBUFFER;
```

打开摄像头后，接下来的任务是查找流信息。以下代码是通过设备获取的内容来自动判断流的信息。这块代码用来判断摄像头/麦克风或文件是没问题的，但如果用来拉流，比如看别人直播，这时候这函数需要执行的时间就很长了，大概5秒左右，那个时候优化的方法是，在自己提前知道了拉流格式后，自己填充流信息。

此处我们打开的设备比较简单，所以简单的使用“查找流信息”函数即可。

```cpp
// 查找流信息
if ((_ret = avformat_find_stream_info (m_ifmt_ctx, nullptr)) < 0) {
    avformat_free_context (m_ifmt_ctx);
    m_ifmt_ctx = nullptr;
    printf ("avformat_find_stream_info failed %d\n", _ret);
    return;
}
```

此处出现了“错误码”这种东西。需要注意的是不是每个FFmpeg函数都有的，遇到错误码通常是将其转为可读的错误信息。具体是否有错误码需要自己去看函数的定义。

```cpp
int _err_code = -1; // 错误码
char _buf [64] = { 0 };
av_strerror (_err_code, _buf, sizeof (_buf));
// 此时 _buf 存储了错误信息
```

然后需要遍历所有流，分别打开解码器。

```cpp
// 遍历解码器
for (unsigned int i = 0; i < m_ifmt_ctx->nb_streams; ++i) {
    AVCodec *_codec = avcodec_find_decoder (m_ifmt_ctx->streams [i]    ->codec->codec_id);
    if (!_codec) {
        avformat_free_context (m_ifmt_ctx);
        m_ifmt_ctx = nullptr;
        printf ("avcodec_find_decoder failed\n");
        return;
    }
    if (avcodec_open2 (m_ifmt_ctx->streams [i]->codec, _codec, nullptr) < 0) {
        avformat_free_context (m_ifmt_ctx);
        m_ifmt_ctx = nullptr;
        printf ("avcodec_open2 failed %d\n", _ret);
        return;
    }
}
```

## 打开输出流

输出流通常需要指定一种包装格式，比如flv、mp4等，如果需要将音视频转为一个文件或者推流则需要指定输出流；如果仅仅展示画面并播放声音那么不需要输出流。

此处以flv作为示例，打开输出流：

```cpp
// 构造输出结构
const char *_fmt_name = "flv"; // 包装格式
const char *_out_url = "D:/a.flv"; // 输出地址，可以是文件，可以是推流地址
int _ret = avformat_alloc_output_context2 (&m_ofmt_ctx, NULL, _fmt_name, _out_url);
if (_ret < 0 || !m_ofmt_ctx) {
    printf ("avformat_alloc_output_context2 failed %d\n", _ret);
    return;
}

// 打开IO输出流
if (!(m_ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
    if ((_ret = avio_open (&m_ofmt_ctx->pb, m_push_url.c_str (), AVIO_FLAG_WRITE)) < 0) {
        printf ("avio_open failed %d\n", _ret);
        return;
    }
}
```

然后需要添加视频流和音频流

```cpp
// 添加视频流
AVCodecID _codec_id = AV_CODEC_ID_H264; // AV_CODEC_ID_AAC
AVCodec *_ocodec = avcodec_find_encoder (_codec_id);
if (!_ocodec) {
    printf ("avcodec_find_encoder failed\n");
    return;
}
AVStream *_ostm = avformat_new_stream (m_ofmt_ctx, _ocodec);
if (!_ostm) {
    printf ("avformat_new_stream failed\n");
    return;
}

//添加音频流类似，只需编码器改一下，这段代码再拷贝一份即可
```

接下来指定音视频流信息，我这里提供音频和视频两个版本。具体参数的值需要根据实际情况来调整，并非完全按照我这儿的设置。

```cpp
// 指定视频编码器信息
AVCodecContext *_codec_ctx = _ostm->codec;// avcodec_alloc_context3 (_ocodec);
_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
_codec_ctx->bit_rate = _height * 1000;
_codec_ctx->width = _width;
_codec_ctx->height = _height;
_codec_ctx->time_base = { 1, 1000 };
_codec_ctx->gop_size = m_fps;
_codec_ctx->max_b_frames = 3;
_codec_ctx->qmin = 10;
_codec_ctx->qmax = 31;
_codec_ctx->framerate = { m_fps, 1 };
_codec_ctx->keyint_min = 25;
_codec_ctx->qcompress = 0.5;
_codec_ctx->qblur = 0.5;
_codec_ctx->max_qdiff = 3;
_codec_ctx->rc_initial_buffer_occupancy = 0;
_codec_ctx->trellis = 0;
_codec_ctx->profile = FF_PROFILE_H264_MAIN;
_codec_ctx->level = 30;
//
//_codec_ctx->me_cmp = 0;
//_codec_ctx->me_subpel_quality = 8;
//_codec_ctx->me_range = 0;
_codec_ctx->chroma_sample_location = AVCHROMA_LOC_LEFT;
_codec_ctx->field_order = AV_FIELD_PROGRESSIVE;
//_codec_ctx->coder_type = FF_CODER_TYPE_VLC;
//_codec_ctx->min_prediction_order = -1;
//_codec_ctx->max_prediction_order = -1;
AVDictionary *_param = nullptr;
if (_codec_ctx->codec_id == AV_CODEC_ID_H264) {
    av_dict_set (&_param, "preset", "ultrafast", 0);
    av_dict_set (&_param, "tune", "zerolatency", 0);
} else if (_codec_ctx->codec_id == AV_CODEC_ID_H265) {
    av_dict_set (&_param, "preset", "ultrafast", 0);
    av_dict_set (&_param, "tune", "zero-latency", 0);
}

// 指定音频编码器信息
uint64_t _channel_layout = AV_CH_LAYOUT_MONO; // 单声道，如果双声道那么 AV_CH_LAYOUT_STEREO
AVCodecContext *_codec_ctx = _ostm->codec;
_codec_ctx->bit_rate = 44100 * 4;
_codec_ctx->bit_rate_tolerance = 4000000;
_codec_ctx->time_base = { 1, 1000 };
_codec_ctx->sample_rate = 44100;
_codec_ctx->channel_layout = _channel_layout;// (_channel_layout > 0 _channel_layout : (_channels > 1 ? AV_CH_LAYOUT_STEREO : AV_CH_FRONT_CENTER));
_codec_ctx->channels = av_get_channel_layout_nb_channels(_codec_ctx->channel_layout);// _channels;
_codec_ctx->sample_fmt = _codec->sample_fmts [0];///AV_SAMPLE_FMT_FLTP;
_codec_ctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
_codec_ctx->keyint_min = 25;
_codec_ctx->qcompress = 0.5;
_codec_ctx->qblur = 0.5;
_codec_ctx->max_qdiff = 3;
_codec_ctx->rc_initial_buffer_occupancy = 0;
_codec_ctx->trellis = 0;
_codec_ctx->profile = FF_PROFILE_AAC_MAIN;
_codec_ctx->level = 30;
AVDictionary *_param = nullptr;
if (_codec_ctx->codec_id == AV_CODEC_ID_AAC) {
    av_dict_set (&_param, "aac_coder", "fast", 0);
}
```

然后需要打开视频流或音频流

```cpp
// 指定header信息，如果不指定可能造成flash等老播放器无法解析播放
if (m_ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
    _ostm->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
_ostm->codec->codec_tag = 0;

// 打开编码器
int _ret = avcodec_open2 (_codec_ctx, _ocodec, &_param);
if (_param)
    av_dict_free (&_param);
if (_ret < 0) {
    printf ("avcodec_open2 failed %d\n", _ret);
    return;
}
if ((_ret = avcodec_parameters_from_context (_ostm->codecpar, _codec_ctx) < 0) {
    printf ("avcodec_parameters_from_context failed %d\n", _ret);
    return;
}
```

## 读帧

此处一次读取的就是一个AVPacket，可以写为循环。如果打开的是设备或流那么不需要考虑暂停；如果打开的是文件那么需要控制读取速度，否则可能会过快。

```cpp
AVPacket *_pkt = av_packet_alloc ();
while (m_run) {
    try {
        // 读一个原始帧包
        av_init_packet (_pkt);
        int _ret = av_read_frame (m_ifmt_ctx, _pkt);
        if (_ret < 0) {
            printf ("av_read_frame failed %d\n", _ret);
            break;
        }
        // 回调、释放、开启下一轮循环
        //_callback (_pkt, _pkt->stream_index);
        av_packet_unref (_pkt);
    } catch (std::exception &e) {
        printf ("catch error: %s", e.what ());
    } catch (...) {
        printf ("catch error");
    }
    // 如果打开的是文件，那么此处需控制速度
    std::this_thread::sleep_for (std::chrono::milliseconds (1));
}
av_packet_free (&_pkt);
```

## 写帧

```cpp
// 初始化设置
int64_t _start_pts = av_gettime ();

// 写帧代码
int _stream_index = 0; //第一个输出流，也就是 avformat_alloc_output_context2 后面第一次调用 avformat_new_stream 创建的流
// 计算duration dts pts
AVRational _time_base = m_fmt_ctx->streams [_stream_index]->time_base;
int64_t _pts = av_rescale_q_rnd (av_gettime () - _start_pts, { 1, AV_TIME_BASE }, _time_base, m_rnd_expr);
if (m_pkt_send->dts < 0 || m_pkt_send->pts < 0 || m_pkt_send->dts > m_pkt_send->pts) {
    m_pkt_send->dts = m_pkt_send->pts = m_pkt_send->duration = 0;
} else {
    m_pkt_send->dts = av_rescale_q_rnd (m_pkt_send->dts, { 1, AV_TIME_BASE }, _time_base, m_rnd_expr);
    m_pkt_send->pts = av_rescale_q_rnd (m_pkt_send->pts, { 1, AV_TIME_BASE }, _time_base, m_rnd_expr);
    m_pkt_send->duration = (int64_t) ((_frame_index + 1) * 1.0 / av_q2d (_time_base) / m_fps) - (int64_t) (_frame_index * 1.0 / av_q2d (_time_base) / m_fps);
}
if (_stream_index == 0)
    _stream_index = _stream_index;

m_pkt_send->pos = -1;
// 此处根据实际情况选择
//if ((_ret = av_interleaved_write_frame (m_ofmt_ctx, m_pkt_send)) < 0) {
if ((_ret = av_write_frame(m_ofmt_ctx, m_pkt_send)) < 0) {
    printf ("av_write_frame(av_interleaved_write_frame) failed %d\n", _ret);
    return false;
}
```

## 程序结构

好了，上面的代码完成了一轮完整的输入输出流操作，不过没有编码解码，数据只能按照原编码进行读写。我们这时候能做的可以是将网红小姐姐直播拉下来然后存文件，或者将一个flv读出来然后写到另一个flv里去等。伪代码如下：

```cpp
#include <头文件>

int main (int argc, char* argv[]) {
    // 初始化
    // 打开输入流
    // 打开输出流
    while (_run) {
        // 从输入流中读一个AVPacket
        // 往输出流中写一个AVPacket
        // 如果读的是文件那么控制速度
    }
    // 关闭输出流
    // 关闭输入流
    return 0;
}
```

其实这时候，你已经能做到将flv1的图像与flv2的声音合并起来存入flv3中了。大胆试试吧

[返回首页](../README.md) | [上一章 FFmpeg 入门](./03_ffmpeg_beginning.md) | [下一章 音视频格式处理](./05_format_process.md)

## 许可

[![test](https://i.creativecommons.org/l/by-nc-nd/4.0/80x15.png)](http://creativecommons.org/licenses/by-nc-nd/4.0/)

本教程采用[知识共享署名-非商业性使用-禁止演绎 4.0 国际许可协议](http://creativecommons.org/licenses/by-nc-nd/4.0/)许可。
