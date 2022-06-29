module.exports = {
    title: 'FawCourse_FFmpeg',
    description: 'FFmpeg教程，非命令行模式',
    themeConfig: {
        nav: [
            { text: 'Github', link: 'https://github.com/fawdlstty/FawCourse_FFmpeg' }
        ],
        sidebar: [{
            title: '首页',
            path: '/',
            children: []
        }, {
            title: '第一章 视频基础',
            path: '/docs/01_video_introduce.md',
            children: [{
                title: '颜色空间',
                path: '/docs/01_video_introduce.md#颜色空间',
                children: []
            }, {
                title: '推荐资料',
                path: '/docs/01_video_introduce.md#推荐资料',
                children: []
            }, {
                title: '视频及图像的黑科技',
                path: '/docs/01_video_introduce.md#视频及图像的黑科技',
                children: []
            }]
        }, {
            title: '第二章 音频基础',
            path: '/docs/02_audio_introduce.md',
            children: [{
                title: '音频采样率',
                path: '/docs/02_audio_introduce.md#音频采样率',
                children: []
            }, {
                title: '声音的黑科技',
                path: '/docs/02_audio_introduce.md#声音的黑科技',
                children: []
            }]
        }, {
            title: '第三章 FFmpeg 入门',
            path: '/docs/03_ffmpeg_beginning.md',
            children: [{
                title: '配置环境',
                path: '/docs/03_ffmpeg_beginning.md#配置环境',
                children: []
            }, {
                title: '容器及编码',
                path: '/docs/03_ffmpeg_beginning.md#容器及编码',
                children: []
            }, {
                title: '各种数据结构的生命周期',
                path: '/docs/03_ffmpeg_beginning.md#各种数据结构的生命周期',
                children: []
            }]
        }, {
            title: '第四章 Hello FFmpeg',
            path: '/docs/04_hello_ffmpeg.md',
            children: [{
                title: '头文件',
                path: '/docs/04_hello_ffmpeg.md#头文件',
                children: []
            }, {
                title: '初始化',
                path: '/docs/04_hello_ffmpeg.md#初始化',
                children: []
            }, {
                title: '打开摄像头及麦克风',
                path: '/docs/docs/04_hello_ffmpeg.md#打开摄像头及麦克风',
                children: []
            }, {
                title: '打开输出流',
                path: '/docs/04_hello_ffmpeg.md#打开输出流',
                children: []
            }, {
                title: '读帧',
                path: '/docs/04_hello_ffmpeg.md#读帧',
                children: []
            }, {
                title: '写帧',
                path: '/docs/04_hello_ffmpeg.md#写帧',
                children: []
            }, {
                title: '程序结构',
                path: '/docs/04_hello_ffmpeg.md#程序结构',
                children: []
            }]
        }, {
            title: '第五章 音视频格式处理',
            path: '/docs/05_format_process.md',
            children: [{
                title: '音视频解码',
                path: '/docs/05_format_process.md#音视频解码',
                children: []
            }, {
                title: '音视频编码',
                path: '/docs/05_format_process.md#音视频编码',
                children: []
            }, {
                title: '音视频格式转换',
                path: '/docs/docs/05_format_process.md#音视频格式转换',
                children: []
            }, {
                title: '音频帧采样数调节',
                path: '/docs/05_format_process.md#音频帧采样数调节',
                children: []
            }, {
                title: '音视频播放',
                path: '/docs/05_format_process.md#音视频播放',
                children: []
            }, {
                title: '程序结构',
                path: '/docs/05_format_process.md#程序结构',
                children: []
            }]
        }, {
            title: '第六章 FFmpeg与其他库的交互',
            path: '/docs/06_interaction.md',
            children: [{
                title: '将avframe与qt互转',
                path: '/docs/06_interaction.md#将avframe与qt互转',
                children: []
            }, {
                title: '将avframe与gdiplus互转',
                path: '/docs/06_interaction.md#将avframe与gdiplus互转',
                children: []
            }, {
                title: '使用sdl1播放视频',
                path: '/docs/docs/06_interaction.md#使用sdl1播放视频',
                children: []
            }, {
                title: '使用sdl2播放视频',
                path: '/docs/06_interaction.md#使用sdl2播放视频',
                children: []
            }, {
                title: '使用sdl2播放音频',
                path: '/docs/06_interaction.md#使用sdl2播放音频',
                children: []
            }, {
                title: '使用wave函数播放音频',
                path: '/docs/06_interaction.md#使用wave函数播放音频',
                children: []
            }, {
                title: '枚举dshow设备',
                path: '/docs/06_interaction.md#枚举dshow设备',
                children: []
            }, {
                title: '使用windows捕获扬声器输出',
                path: '/docs/06_interaction.md#使用windows捕获扬声器输出',
                children: []
            }, {
                title: '程序结构',
                path: '/docs/06_interaction.md#程序结构',
                children: []
            }]
        }, {
            title: '第七章 Filter 滤镜',
            path: '/docs/07_filter.md',
            children: [{
                title: '非滤镜图像处理',
                path: '/docs/07_filter.md#非滤镜图像处理',
                children: []
            }, {
                title: '非滤镜音频处理',
                path: '/docs/07_filter.md#非滤镜音频处理',
                children: []
            }, {
                title: '滤镜的使用',
                path: '/docs/07_filter.md#滤镜的使用',
                children: []
            }]
        }, {
            title: '第八章 硬编码加速',
            path: '/docs/08_hard.md',
            children: [{
                title: '为什么需要硬编码',
                path: '/docs/08_hard.md#为什么需要硬编码',
                children: []
            }, {
                title: '编译intel硬编码',
                path: '/docs/08_hard.md#编译intel硬编码',
                children: []
            }, {
                title: '编译nvidia硬编码',
                path: '/docs/08_hard.md#编译nvidia硬编码',
                children: []
            }, {
                title: '编译amd硬编码',
                path: '/docs/08_hard.md#编译amd硬编码',
                children: []
            }]
        }, {
            title: '第九章 常见问题',
            path: '/docs/09_FAQ.md',
            children: []
        }, {
            title: '附录1 人脸检测',
            path: '/docs/a1_face_detect.md',
            children: [{
                title: '人脸检测原理',
                path: '/docs/a1_face_detect.md#人脸检测原理',
                children: []
            }, {
                title: '人脸检测实现',
                path: '/docs/a1_face_detect.md#人脸检测实现',
                children: []
            }, {
                title: '人脸贴图',
                path: '/docs/a1_face_detect.md#人脸贴图',
                children: []
            }]
        }, {
            title: '附录2 流媒体服务器',
            path: '/docs/a2_media_server.md',
            children: [{
                title: '流媒体协议',
                path: '/docs/a2_media_server.md#流媒体协议',
                children: []
            }, {
                title: '搭建流媒体服务器',
                path: '/docs/a2_media_server.md#搭建流媒体服务器',
                children: []
            }]
        }]
    }
}

// npm run docs:dev
// npm run docs:build
