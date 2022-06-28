# FawCourse_FFmpeg

FFmpeg教程，非命令行模式

[GitHub地址](https://github.com/fawdlstty/FawCourse_FFmpeg)

[在线阅读](https://www.fawdlstty.com/ffmpeg/index.html)

[所有示例](https://github.com/fawdlstty/FawCourse_FFmpeg/tree/master/example)

## 引言

FFmpeg 是一个音视频领域使用最广泛的开源库。由C语言编写，但广泛的为C/C++、C#、Java、Python等主流编程语言所调用。它集合了几乎所有的编码解码库与流协议，并能任意添加图片或文字水印，几乎能完美处理您对音视频开发领域的几乎所有需求。
FFmpeg的开源协议为LGPL或GPL协议，也就是说它能在一定程度上允许闭源商用，前提是不要使用它的GPL开源的功能。

## 起因

FFmpeg算是我个人近期入门时间最长的一个库了，学它耗费了我大量时间，最主要的原因还是因为资料难找。所以我个人整理一套FFmpeg入门资料，留作备用，另外也帮助后面准备入门的初学者们能更快的上手开发。

我对FFmpeg的入门离不开雷总博客的帮助。在此向雷总致敬。

雷霄骅的博客：[https://blog.csdn.net/leixiaohua1020](https://blog.csdn.net/leixiaohua1020)

雷霄骅的代码库：[https://github.com/leixiaohua1020](https://github.com/leixiaohua1020)

## 目标读者

1. 本教程假定读者已经熟悉了传统 C/C++ ，至少在阅读传统 C++ 代码上不具备任何困难，渴望在短时间内迅速了解**音视频**原理的人或需要对**音视频**做处理的人非常适合；
2. 本教程一定程度上整理了音视频领域开发的关键点，对FFmpeg开发比较熟悉，需要一本方便随时查阅的读者。

## 目录

- [介绍](README.md)
- [**第一章 视频基础**](zh_hans/01_video_introduce.md)
    + [颜色空间](zh_hans/01_video_introduce.md#颜色空间)
    + [推荐资料](zh_hans/01_video_introduce.md#推荐资料)
    + [视频及图像的黑科技](zh_hans/01_video_introduce.md#视频及图像的黑科技)
- [**第二章 音频基础**](zh_hans/02_audio_introduce.md)
    + [音频采样率](zh_hans/02_audio_introduce.md#音频采样率)
    + [声音的黑科技](zh_hans/02_audio_introduce.md#声音的黑科技)
- [**第三章 FFmpeg 入门**](zh_hans/03_ffmpeg_beginning.md)
    + [配置环境](zh_hans/03_ffmpeg_beginning.md#配置环境)
    + [容器及编码](zh_hans/03_ffmpeg_beginning.md#容器及编码)
    + [各种数据结构的生命周期](zh_hans/03_ffmpeg_beginning.md#各种数据结构的生命周期)
- [**第四章 Hello FFmpeg**](zh_hans/04_hello_ffmpeg.md)
    + [头文件](zh_hans/04_hello_ffmpeg.md#头文件)
    + [初始化](zh_hans/04_hello_ffmpeg.md#初始化)
    + [打开摄像头及麦克风](zh_hans/04_hello_ffmpeg.md#打开摄像头及麦克风)
    + [打开输出流](zh_hans/04_hello_ffmpeg.md#打开输出流)
    + [读帧](zh_hans/04_hello_ffmpeg.md#读帧)
    + [写帧](zh_hans/04_hello_ffmpeg.md#写帧)
    + [程序结构](zh_hans/04_hello_ffmpeg.md#程序结构)
- [**第五章 音视频格式处理**](zh_hans/05_format_process.md)
    + [音视频解码](zh_hans/05_format_process.md#音视频解码)
    + [音视频编码](zh_hans/05_format_process.md#音视频编码)
    + [音视频格式转换](zh_hans/05_format_process.md#音视频格式转换)
    + [音频帧采样数调节](zh_hans/05_format_process.md#音频帧采样数调节)
    + [音视频播放](zh_hans/05_format_process.md#音视频播放)
    + [程序结构](zh_hans/05_format_process.md#程序结构)
- [**第六章 FFmpeg与其他库的交互**](zh_hans/06_interaction.md)
    + [将avframe与qt互转](zh_hans/06_interaction.md#将avframe与qt互转)
    + [将avframe与gdiplus互转](zh_hans/06_interaction.md#将avframe与gdiplus互转)
    + [使用sdl1播放视频](zh_hans/06_interaction.md#使用sdl1播放视频)
    + [使用sdl2播放视频](zh_hans/06_interaction.md#使用sdl2播放视频)
    + [使用sdl2播放音频](zh_hans/06_interaction.md#使用sdl2播放音频)
    + [使用wave函数播放音频](zh_hans/06_interaction.md#使用wave函数播放音频)
    + [枚举dshow设备](zh_hans/06_interaction.md#枚举dshow设备)
    + [使用windows捕获扬声器输出](zh_hans/06_interaction.md#使用windows捕获扬声器输出)
    + [程序结构](zh_hans/06_interaction.md#程序结构)
- [**第七章 Filter 滤镜**](zh_hans/07_filter.md)
    + [非滤镜图像处理](zh_hans/07_filter.md#非滤镜图像处理)
    + [非滤镜音频处理](zh_hans/07_filter.md#非滤镜音频处理)
    + [滤镜的使用](zh_hans/07_filter.md#滤镜的使用)
- [**第八章 硬编码加速**](zh_hans/08_hard.md)
    + [为什么需要硬编码](zh_hans/08_hard.md#为什么需要硬编码)
    + [编译intel硬编码](zh_hans/08_hard.md#编译intel硬编码)
    + [编译nvidia硬编码](zh_hans/08_hard.md#编译nvidia硬编码)
    + [编译amd硬编码](zh_hans/08_hard.md#编译amd硬编码)
- [**第九章 常见问题**](zh_hans/09_FAQ.md)
- [**附录1 人脸检测**](zh_hans/a1_face_detect.md)
    + [人脸检测原理](zh_hans/a1_face_detect.md#人脸检测原理)
    + [人脸检测实现](zh_hans/a1_face_detect.md#人脸检测实现)
    + [人脸贴图](zh_hans/a1_face_detect.md#人脸贴图)
- [**附录2 流媒体服务器**](zh_hans/a2_media_server.md)
    + [流媒体协议](zh_hans/a2_media_server.md#流媒体协议)
    + [搭建流媒体服务器](zh_hans/a2_media_server.md#搭建流媒体服务器)


## 其他

开源协议：CC BY-NC-ND 4.0

![打赏](./donate.jpg)
