# 第三章 FFmpeg 入门

本章开始正式入门FFmpeg，我将对FFmpeg环境配置、主要编码及基本数据类型做出详细的介绍。

## 配置环境

在Win32环境配置FFmpeg开发有一个非常简单的方法：

1. 安装Visual Studio 2017，并安装英文语言包
2. 安装vcpkg，并执行vcpkg install ffmpeg*:x86-windows ffmpeg*:x86-windows-static
3. 此时新建一个项目即可直接使用FFmpeg

其他方式配置稍显麻烦，此处暂时不列出

另外还需要安装一个FFmpeg命令行环境，推荐官网直接下一个，需要注意的是下载的是GPL协议的开发库。

## 容器及编码

### 容器

容器通常是一种文件格式或流媒体格式，用来描述纯视频、纯音频或音视频的存储格式。

纯音频容器格式有mp3、aac、wma等，通常你放的歌均使用这种容器来存储。

纯视频容器格式我暂时没见过，可能有。

混合容器格式有mp4、flv、vob、rmvb等，通常看的视频，音视频都有的格式就使用这种存储格式，它们除了能混合存储，还能仅存放音频或视频。比如一首mp3的歌转为flv格式来播放完全是没问题的。

### 图像编码

未压缩编码格式有rgb/bgr、hsl/hsv/hsb、yuv等等系列，因体积因素，使用最广泛的是yuv系列。这类编码方式有一个共同的特点，也就是抽取其中任意一帧都能表述整个图像。

压缩编码格式有h263/h264/h265、vp8/vp9/av1、flv1等等，这类图像均经过压缩，只有I帧能完整还原整个画面（前提是知道metadata信息），其他类型的帧可能是在I帧基础上做一些改动的描述，比如有了I帧后，新的帧可以对其做出一定的修改；也可能压根不存储画面信息，只存放metadata或SEI信息（h264/h265）等。

### 音频编码

未压缩的音频编码有两个主要概念，一个是格式，一个是采样率。通常格式是使用16位有符号数字作为采样，取值范围-32768~32767，网上看到的很多音频处理软件的声音波形图就是这个值；除了这个外，还能用8位整数、32位整数、32位float来表示，存储容量越大，声音也就越保真。通常16位能满足绝大多数人对声音需求，所以很少有场景使用32位无损音质。同时32位也是人类对声音分辨的极限，超过32位的比如64位音质，一方面没任何优势，另一方面浪费存储空间。

采样率的意思是每秒钟存储多少个声音的值，主流使用44.1KHz，也就是每秒钟有44100个采样点，能满足绝大多数人的需求了。同时人类对采样率识别的极限是48KHz，也就是每秒钟有48000个采样点。超过这个频率的音质对人类来说没有任何区别，除非特殊场合，否则48KHz完全足够，主流依然是44.1KHz。

## 各种数据结构的生命周期

FFmpeg对象的生命周期管理也是非常重要的内容，这一块如果不处理好非常容易导致内存泄露等情况。我个人习惯的方式是使用Visual Studio2017开发，里面有一款小工具Diagnostic Tools，可以很方便直观的展示内存及CPU使用率动态变化情况。FFmpeg一大难点也是这个，非常容易忽略生命周期的管理，从而导致内存泄露，在没有这工具的情况下很难排查具体泄漏点。这一章将直观的讲述生命周期的管理，配合内存监测类工具，可以很容易排查内存泄露的原因。

### AVCodec

编码对象，通常通过 avcodec_find_decoder 函数，传入编码ID获得，可以直接抛弃，不用关心泄露。

### AVFormatContext

最重要的对象之一，用于维护一个输入流或输出流，通常通过 avformat_alloc_output_context2 函数打开，由 avformat_close_input 函数释放。对于文件对象，生命周期类似文件句柄；对于网络IO对象，生命周期类似C语言网络函数中的SOCKET句柄。

### AVStream

这个类的作用就是指定具体的流的类型，比如一个MP4文件有音视频数据，那么处理这个文件就由一个 AVFormatContext 对象来维护，可以通过这个对象访问两个 AVStream 对象，一个用于视频的处理（读或写），一个用于音频的处理（读或写）

在读流的情况下，使用 avformat_find_stream_info 找出所有流的信息同时打开所有流，生命周期此时将交于 AVFormatContext 对象托管，不用再关心泄露。

在写流的情况下，使用 avformat_new_stream 创建新的流并关联至 AVFormatContext，生命周期此时将交于 AVFormatContext 对象托管，不用再关心泄露。

### AVCodecContext

用于将数据进行编解码，比如将yuv420p编码为h264数据，或者将aac数据解码为fltp格式数据，音视频编解码全靠这个类。

它可以独立存在，也能与 AVStream 相关联；通常一个 AVStream 对象就有一个 AVCodecContext。当它与 AVStream 关联后，生命周期与 AVStream 一样，交给 AVFormatContext 对象管理。

通常写流时通过制定参数，然后通过 avcodec_open2 打开编码；读流时在通过 avcodec_find_decoder 找到 AVCodec* 后，通过 avcodec_open2 打开编码。

如果它没有与 AVStream 相关联，那么需要手动关闭，先 avcodec_close，再 avcodec_free_context。

### AVInputFormat

用于查找输入流的对象，在通过 avformat_open_input 使用后，生命周期将交于 AVFormatContext 对象托管，不用再关心泄露。

### AVDictionary

通常用于打开前指定参数，使用 av_dict_set 函数设置；使用完毕后通过 av_dict_free 函数释放。

### AVPacket

一个这个结构代表一个数据包，用于存储编码后的数据，比如h264 raw数据或aac raw数据。这个数据结构由两部分组成，结构本身和数据部分。

结构本身通过 av_packet_alloc 与 av_packet_free 分配及释放；数据部分通常通过 avcodec_receive_packet 或 av_read_frame 也就是编码后或者从流管道获取一帧数据，由于可能有多个 AVPacket 引用同一块数据，所以不能直接释放，需使用 av_packet_unref 结束引用这一块数据，如果没有结构再引用数据后，数据内存区域将自动释放。

### AVFrame

用于储存音视频的一帧数据，可以储存视频图像的rgb或yuv像素格式数据，也可以储存音频的s16或fltp采样格式数据，其中音频一帧的采样数与时长是不定的，一帧可能有几十毫秒时长的数据，也可能有半秒时长的数据。

结构本身通过 av_frame_alloc 与 av_frame_free 分配及释放；数据部分通常通过 avcodec_receive_frame 或 av_frame_get_buffer 解码 AVPacket 或者自己分配。同ACPacket一样，由于可能有多个 AVFrame 引用同一块数据，所以不能直接释放，需使用 av_frame_unref 结束引用这一块数据，如果没有结构再引用数据后，数据内存区域将自动释放。
