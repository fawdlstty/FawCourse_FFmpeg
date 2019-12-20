# 第七章 Filter 滤镜

在讲述这章前，我先提一个问题：如何给视频加水印？

Windows客户端开发的小伙伴第一个想到的是GDI+吧？视频读入GDI+，然后每帧画一个水印，再写入新的视频中；Qt也一样，skia等其他绘图库同理。

我个人这儿是不太推荐使用滤镜的，滤镜难学，并且很容易出问题，如果对其他绘图库比较熟悉了，并且能解决问题了，那么尽量就不要用滤镜。滤镜相对于其他库来说效率并不会提高多少，唯一好处就是，不用附带其他绘图库，利于软件的三方库的统一。

## 非滤镜图像处理

这个比较简单，以GDI+为例：

```cpp
// AVFrame* 转 rgb32格式AVFrame*
// rgb32格式AVFrame* 转 Gdiplus::Bitmap
// 对Gdiplus::Bitmap进行绘制
// Gdiplus::Bitmap 转 rgb32格式AVFrame*
// rgb32格式AVFrame* 转 AVFrame*
```

## 非滤镜音频处理

需要达到很棒的效果的音频处理需要加很多音频公式，比如去噪等等。这儿我给出一个示例，双路音频合流，其他关于音频的处理同理。

处理音频首先需要将各种格式统一，比如channel都转为1，然后sample_rate都转为44100，音频格式都转为`AV_SAMPLE_FMT_S16`，也就是short类型数据，有符号16位数字，取值范围为-32768~32767。

然后开始合流，对S16格式音频来说，合流就是简单的相加即可。

```cpp
AVFrame *_frame1, *_frame2;// 这两帧为有效的 AV_SAMPLE_FMT_S16 格式，单声道，相同采样率
size_t _sCount = _frame1->nb_samples;
int16_t *_data1 = (int16_t*) _frame1->data [0];
int16_t *_data2 = (int16_t*) _frame2->data [0];
for (size_t i = 0; i < _sCount; ++i, ++_data1, ++_data2) {
    int _n = *_data1 + (int) *_data2;
    _n = (_n < -32768 ? -32768 : _n);
    _n = (_n > 32767 ? 32767 : _n);
    // 此处将结果放置在_frame1中
    *_data1 = (int16_t) _n;
}
```

## 滤镜的使用

虽然我个人不推荐使用滤镜，不过如果有想使用滤镜的，那么按照下面步骤来：

```cpp
// 初始化
AVFilterGraph *_graph = avfilter_graph_alloc ();
// 生成描述字符串
char _args [512] = { '\0' };
snprintf (_args, _countof (_args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", _codec_ctx->width, _codec_ctx->height, AV_PIX_FMT_YUV420P, _codec_ctx->time_base.num, _codec_ctx->time_base.den, _codec_ctx->sample_aspect_ratio.num, _codec_ctx->sample_aspect_ratio.den); // 视频
snprintf (_args, _countof (_args), "sample_rate=%d:sample_fmt=%s:time_base=%d/%d:channel_layout=%I64x", _codec_ctx->sample_rate, av_get_sample_fmt_name (AV_SAMPLE_FMT_FLTP), _codec_ctx->time_base.num, _codec_ctx->time_base.den, _codec_ctx->channel_layout); // 音频
// 创建输入信息结构
const AVFilter *_filter_i avfilter_get_by_name ("buffer"); // 视频
const AVFilter *_filter_i avfilter_get_by_name ("abuffer"); //音频
AVFilterContext *_filter_ctx_i = nullptr;
int _ret = avfilter_graph_create_filter (&_filter_ctx_i, m_filters_i [_n], _name.c_str (), _args, nullptr, m_graph);
if (!_filter_ctx_i) {
    printf ("avfilter_graph_create_filter failed %d\n", _ret);
    return false;
}
// 创建输入管道
AVFilterInOut *_output = avfilter_inout_alloc ();
_output->name = av_strdup (_name.c_str ());
_output->filter_ctx = _filter_ctx_i;
_output->pad_idx = 0;
_output->next = nullptr;
// 创建输出信息结构
const AVFilter *_filter_o avfilter_get_by_name ("buffersink"); // 视频
const AVFilter *_filter_o avfilter_get_by_name ("abuffersink"); //音频
AVFilterContext *_filter_ctx_o = nullptr;
_ret = avfilter_graph_create_filter (&_filter_ctx_o, m_filter_o, "out", nullptr, nullptr, m_graph);
if (!_filter_ctx_o) {
    printf ("avfilter_graph_create_filter failed %d\n", _ret);
    return false;
}
// 创建输出管道
AVFilterInOut *_input = avfilter_inout_alloc ();
_input->name = av_strdup ("out");
_input->filter_ctx = m_filter_ctx_o;
_input->pad_idx = 0;
_input->next = nullptr;
// 指定输出像素格式或采样格式
// 视频
enum AVPixelFormat pix_fmts [] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
_ret = av_opt_set_int_list (m_filter_ctx_o, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
// 音频
static AVSampleFormat _sample_fmt = AV_SAMPLE_FMT_FLTP;
_ret = av_opt_set_bin (m_filter_ctx_o, "sample_fmts", (uint8_t*) &_sample_fmt, sizeof (_sample_fmt), AV_OPT_SEARCH_CHILDREN);
_ret = av_opt_set_bin (m_filter_ctx_o, "channel_layouts", (uint8_t*) &m_channel_layout, sizeof (m_channel_layout), AV_OPT_SEARCH_CHILDREN);
_ret = av_opt_set_bin (m_filter_ctx_o, "sample_rates", (uint8_t*) &m_sample_rate, sizeof (m_sample_rate), AV_OPT_SEARCH_CHILDREN);
// 指定滤镜语法
const char *_filter_str = "[in]crop=640:420[out]"; // 此处为滤镜语法
if ((_ret = avfilter_graph_parse_ptr (m_graph, _afilter.c_str (), &_input, _outputs, nullptr)) < 0) {
    printf ("avfilter_graph_parse_ptr failed %d\n", _ret);
    return false;
}
if ((_ret = avfilter_graph_config (m_graph, nullptr)) < 0) {
    printf ("avfilter_graph_config failed %d\n", _ret);
    return false;
}

// 下面开始循环处理数据
// 写入
_ret = av_buffersrc_add_frame_flags (_filter_ctx_i, _frame_new, AV_BUFFERSRC_FLAG_KEEP_REF);
// 读取
_ret = av_buffersink_get_frame (_filter_ctx_o, _frame);

// 释放
avfilter_free (_filter_ctx_i);
_filter_ctx_i = nullptr;
avfilter_free (_filter_ctx_o);
_filter_ctx_o = nullptr;
av_free ((void*) _filter_i);
_filter_i = nullptr;
av_free ((void*) _filter_o);
_filter_o = nullptr;
avfilter_graph_free (&_graph);
```

<!--
## 滤镜语法

滤镜的实现是通过滤镜语法来的，所以再实现滤镜前先说说滤镜语法。这种语法实际上是一种DSL，专为FFmpeg滤镜服务。

### 文字水印

|参数|说明|示例值|
|:-:|:-|:-|
|box|是否显示外边框，默认0|1、0|
|boxborderw|外边框宽度，默认0|0、1、2、……|
|boxcolor|外边框颜色（[0x\|#]RRGGBB[AA]、[颜色名称](https://ffmpeg.org/ffmpeg-utils.html#Color)），默认white|red、0x112233、#44556677|
|line_spacing|行间距，默认0|0、1、2、……|
|borderw|边框宽度，默认0|0、1、2、……|
|bordercolor|边框颜色（[0x\|#]RRGGBB[AA]、[颜色名称](https://ffmpeg.org/ffmpeg-utils.html#Color)），默认black|red、0x112233、#44556677|
|expansion|文本展开方式|none、normal、~~strftime~~|
|~~basetime~~|以微秒为单位设定开始时间，仅expansion为~~strftime~~时生效。正常方式模拟这个效果使用pts函数|0|
|fix_bounds|如果为1，则检查修复文本避免被剪切|1、0|
|fontcolor|字体颜色（[0x\|#]RRGGBB[AA]、[颜色名称](https://ffmpeg.org/ffmpeg-utils.html#Color)），默认black|red、0x112233、#44556677|
|fontcolor_expr|以与文本相同方式扩展字符串，如果不为空则覆盖fontcolor属性|#%H%m%s|
|font|指定绘制文本的默认字体，默认没有|consolas|
|fontfile|指定字体文件路径，如果禁用fontconfig则必须包含此参数|test.ttf|
|alpha|透明度，[0,1]区间的数字，默认1|0.3、0.7|
|fontsize|字体大小，默认16|12、24|
|text_shaping|如果为1，则在绘制之前尝试对文本进行塑形，比如RTL等，默认1|1、0|
|ft_load_flags|default(默认)、no_scale(无缩放)、no_hinting(无提示)、render(渲染)、no_bitmap(无位图)、vertical_layout(垂直布局)、force_autohint(强制自动暗示)、crop_bitmap(裁剪位图)、pedantic(迂腐)、ignore_global_advance_width(忽略全局推进宽度)、no_recurse(无递归)、ignore_transform(忽略变换)、monochrome(单色)、linear_design(线性设计)、no_autohint(无自动暗示)|render|
|shadowcolor|文字阴影色（[0x\|#]RRGGBB[AA]、[颜色名称](https://ffmpeg.org/ffmpeg-utils.html#Color)），默认black|red、0x112233、#44556677|
|shadowx<br/>shadowy|阴影偏移，默认0|0、1、2……|
|start_number|起始帧号，默认0|0、1、2……|
|tabsize|制表符宽度，默认4|4、8、……|
|timecode|设置初始化时间代码，使用它必须同时指定timecode_rate选项|hh:mm:ss[:;.]ff|
|timecode_rate<br/>rate<br/>r|最小值为1，drop-frame支持帧率为30和60|30|
|tc24hmax|如果设置为1，那么时间点围绕24小时。默认0|0、1|
|text|文字内容|hello world|
|textfile|字符串文件路径，与text参数只能有且仅有一个存在|test.txt|
|reload|如果为1，那么字符串文件将在每帧加载，以确保自动更新|1,0|
|x<br/>y|指定文本偏移的表达式，默认0|0、t / h|

参数介绍的差不多了，下面来实际写一串文本滤镜：

```
drawtext=fontsize=100:fontfile=a.ttf:text='hello':x=20:y=20
```

这一行中分为两部分，一部分是“`drawtext=`”，这部分指定绘制文本参数，如果有多个指定项则用`;`分割；另一部分是“`fontsize=100:fontfile=a.ttf:text='hello':x=20:y=20`”，全是drawtext的内容，这部分分隔符是“`:`”。

文本或者动态颜色字符串可以使用格式化方式来动态生成，比如

```
...text='%{localtime\:%Y-%m-%d %H\:%M\:%S}'...
```
-->

参考：[ffmpeg-filters](https://ffmpeg.org/ffmpeg-filters.html)

[返回首页](../README.md) | [上一章 FFmpeg与其他库的交互](./06_interaction.md) | [下一章 硬编码加速](./08_hard.md)

## 许可

[![test](https://i.creativecommons.org/l/by-nc-nd/4.0/80x15.png)](http://creativecommons.org/licenses/by-nc-nd/4.0/)

本教程采用[知识共享署名-非商业性使用-禁止演绎 4.0 国际许可协议](http://creativecommons.org/licenses/by-nc-nd/4.0/)许可。
