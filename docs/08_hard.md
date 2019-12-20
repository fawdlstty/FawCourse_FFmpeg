# 第八章 硬编码加速

## 为什么需要硬编码

想得到较高性能的压缩率，或者更高清的性能，势必会需要更多的计算。当CPU性能带不过来时，就得考虑硬编方案了。

硬编方式与普通方式有

目前已知有三种硬编加速方式：Intel qsv 指令加速、NVIDIA 显卡加速以及 AMD 显卡加速。

FFmpeg编译一直是难上加难，对于不会编译的开发者们来说最方便的方式莫过于使用编译好的库（如果没有找到最合适自己的硬编方式那么需要手工编译了）：  
[部分收集的资源的资源](../assets/build/README.md)

## Intel QSV 指令加速

这个应该算兼容性最强的编码方式了，在绝大部分intel cpu上都能运行，但有个问题是，只能按照特定方式编码，并未支持所有的h264编码方式，具体验证是否支持需手工尝试。

编码器/解码器：h264_qsv

编译方式：（雾）

1. 安装：mfx_dispatcher
2. configure 新增命令行：`--enable-libmfx --enable-encoder=h264_qsv --enable-decoder=h264_qsv`

## NVIDIA 显卡加速

这个应该算综合指数最佳的编码方式，有条件选N卡硬编那就选N卡硬编，一方面用户量大，另一方面效果非常好。

编码器/解码器：h264_nvenc、nvenc_h264、nvenc

编译方式：

1. 下载 CUDA SDK，链接：<https://developer.nvidia.com/cuda-toolkit-archive>  
  不同版本对应不同的驱动版本支持时间，比如 `CUDA Toolkit 9.2 (May 2018)` 代表用户在2018年5月后更新过显卡驱动那么都能支持；在此之前更新显卡驱动都无法支持  
  然后下载对应操作系统版本，Win7或者Win8.1或者Win10，这个代表当前开发机的操作系统版本，假如在Win10上面开发并下载Win10的SDK，开发出的软件依旧可以在装了驱动的Win7电脑上运行  
  最新的SDK版本编译最简单
2. 安装vcpkg，并 install `ffnvcodec`
3. 截至目前 vcpkg 还不直接支持 nvenc 硬编编译（如果某天支持后可以省略这一步），可以参考这个PR的修改：<https://github.com/microsoft/vcpkg/pull/9171>  
  如果是选择的较老 CUDA SDK，那么需要找到对应的 ffnvcodec，链接：<https://github.com/FFmpeg/nv-codec-headers/blob/250292dd20af60edc6e0d07f1d6e489a2f8e1c44/include/ffnvcodec/nvEncodeAPI.h#L117>  
  确保某个历史版本的 ffnvcodec 的这个宏的值低于 CUDA SDK 的版本，然后再将 ffmpeg 大版本从 4.x.x 降级为 3.x.x
4. 使用vcpkg 命令编译

## AMD 显卡加速

在我看来这是一种对于 NVIDIA 显卡加速的一种补充方式，它可以使得支持的用户更广，但 AMD 显卡的用户量比 NVIDIA 显卡用户量小得多，所以这种方式一般作为最后的支持选项。当然有条件全部硬编支持是最好的

编码器/解码器：h264_amf

编译方式：（大雾）

[返回首页](../README.md) | [上一章 Filter 滤镜](./07_filter.md) | [下一章 常见问题](./09_FAQ.md)

## 许可

[![test](https://i.creativecommons.org/l/by-nc-nd/4.0/80x15.png)](http://creativecommons.org/licenses/by-nc-nd/4.0/)

本教程采用[知识共享署名-非商业性使用-禁止演绎 4.0 国际许可协议](http://creativecommons.org/licenses/by-nc-nd/4.0/)许可。
