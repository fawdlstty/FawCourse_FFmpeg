# FawCourse_FFmpeg

FFmpeg教程，非控制台模式

[所有示例](example/README.md)

## 目录

- [**序言**](docs/00_startup.md)
    + 引言
    + 起因
    + 目标读者

- [**第一章 视频基础**](docs/01_video_introduce.md)
    + 颜色空间
        * RGB颜色空间
        * YUV颜色空间
    + 推荐资料
    + 视频/图像的黑科技
- [**第二章 音频基础**](docs/02_audio_introduce.md)
    + 音频采样率
    + 声音的黑科技
- [**第三章 FFmpeg 入门**](docs/03_ffmpeg_beginning.md)
    + 配置环境
    + 容器及编码
        * 容器
        * 图像编码
        * 音频编码
    + 各种数据结构的生命周期
        * AVCodec
        * AVFormatContext
        * AVStream
        * AVCodecContext
        * AVInputFormat
        * AVDictionary
        * AVPacket
        * AVFrame
- [**第四章 Hello FFmpeg**](docs/04_hello_ffmpeg.md)
    + 头文件
    + 初始化
    + 打开摄像头/麦克风
    + 打开输出流
    + 读帧
    + 写帧
    + 程序结构

- [**第五章 音视频格式处理**](docs/05_format_process.md)
    + 音视频解码
    + 音视频编码
    + 音视频格式转换
    + 音频帧采样数调节
    + 程序结构

- [**第六章 FFmpeg与其他库的交互**](docs/06_interaction.md)
    + AVFrame与Qt图片互转
    + AVFrame与GDI+图片互转
    + SDL1播放视频
    + SDL2播放视频
    + SDL2播放音频
    + wave函数播放音频
    + 枚举dshow设备
    + Windows捕获扬声器输出
    + 程序结构

- [**第七章 Filter 滤镜**](docs/07_filter.md)
    + 非滤镜图像处理
    + 非滤镜音频处理
    + 滤镜的使用

## 许可

[![test](https://i.creativecommons.org/l/by-nc-nd/4.0/80x15.png)](http://creativecommons.org/licenses/by-nc-nd/4.0/)

本教程采用[知识共享署名-非商业性使用-禁止演绎 4.0 国际许可协议](http://creativecommons.org/licenses/by-nc-nd/4.0/)许可。
