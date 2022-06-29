# 附录2 流媒体服务器

## 流媒体协议

流媒体协议通常分为两大类，直播协议与点播协议。直播就是那种网红小姐姐或者在线玩游戏的，点播类似爱奇艺那种看电影的。比如：

- rtmp：使用最广泛的流媒体协议
- rtsp/rtp：与rtmp类似的技术，但支持不同的流格式
- udp：低延迟协议
- rtmpe：加密rtmp格式
- http(m3u8)：视频点播协议
- webrtc：视频聊天，使用打洞技术，两台电脑直接通信

## 搭建流媒体服务器

此处我列举几个有代表性的协议的服务端部署方式

- rtmp
	+ <https://github.com/ossrs/srs>
	+ <https://github.com/arut/nginx-rtmp-module>
	+ <https://github.com/illuspas/nginx-rtmp-win32>
	+ <https://github.com/rgaufman/live555>
- webrtc
	+ <https://webrtc.org.cn/mirror/>
		* 备注：这玩意虽然有国内镜像，但编译时还得科学上网才行
	+ <https://github.com/flutter-webrtc/flutter-webrtc>

`nginx-rtmp-win32`这个最简单，它是一个Windows端编译好的程序，克隆下来直接运行就行了，非常适合用来测试。

然后webrtc这个，虽然是国内镜像，但编译依旧需要科学上网才行，我暂时还没找到不科学的编译方式。不过只看安卓和iOS那是有的，flutter-webrtc可以，截至目前它还没支持Windows端，希望再过不久能支持吧。另外搭建它的服务器不是用于流媒体的传输，更多的是用于命令的交换以及打洞的需要。

至于m3u8服务器搭建，这个就简单了，网上随便找个nginx启用一个网站，将m3u8文件与ts视频文件传到网站目录下，就可以了。其中ts是一种封装的视频格式，m3u8是一个文本文件。

m3u8格式介绍：<https://blog.csdn.net/kl222/article/details/14526031>
