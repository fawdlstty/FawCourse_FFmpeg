# 第六章 FFmpeg与其他库的交互

视频软件不是仅靠一个FFmpeg就能完成。FFmpeg只包括流的读写及编码解码，其他操作都得靠其他库来实现。比如播放视频需要界面库；播放声音需要播放声音的库或系统API等。所以这一块也是重中之重。下面我介绍几种常用FFmpeg与其他库交互方式。

## AVFrame与Qt图片互转

AVFrame转QImage比较简单，不过这儿有一个坑在于，直接指定数据的方式上面，一旦数据失效那么图片也会失效。此处的比较友好的做法是再构造一次

```cpp
AVFrame *_frame_rgb32 = _video_format_convet (_frame, AV_PIX_FMT_RGB32);
// 此处_img_tmp即可用，生命周期与_frame_rgb32相同
QImage _img_tmp (_frame_rgb32->data [0], _frame_rgb32->width, _frame_rgb32->height, QImage::Format_RGB32);

//但如果想让_frame_rgb32释放后继续使用，那么需要再构造一次
QImage _img { 640, 480, QImage::Format_ARGB32 };
QPainter _p (&_img);
_p.drawImage (_img.rect (), _img_tmp, _img_tmp.rect ());
```

QImage转AVFrame也比较容易，这个过程反过来即可

```cpp
```

## AVFrame与GDI+图片互转

```cpp
```

## SDL2播放视频

```cpp
```

## SDL2播放音频

```cpp
```

## wave函数播放音频

```cpp
```

[返回首页](../README.md) | [上一章 音视频格式处理](./05_format_process.md) | 下一章

## 许可

[![test](https://i.creativecommons.org/l/by-nc-nd/4.0/80x15.png)](http://creativecommons.org/licenses/by-nc-nd/4.0/)

本教程采用[知识共享署名-非商业性使用-禁止演绎 4.0 国际许可协议](http://creativecommons.org/licenses/by-nc-nd/4.0/)许可。
