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
            title: '第一章：视频基础',
            path: '/docs/01_video_introduce/'
        }, {
            title: '第二章 音频基础',
            path: '/docs/02_audio_introduce/'
        }, {
            title: '第三章 FFmpeg 入门',
            path: '/docs/03_ffmpeg_beginning/'
        }, {
            title: '第四章 Hello FFmpeg',
            path: '/docs/04_hello_ffmpeg/'
        }, {
            title: '第五章 音视频格式处理',
            path: '/docs/05_format_process/'
        }, {
            title: '第六章 FFmpeg与其他库的交互',
            path: '/docs/06_interaction/'
        }, {
            title: '第七章 Filter 滤镜',
            path: '/docs/07_filter/'
        }, {
            title: '第八章 硬编码加速',
            path: '/docs/08_hard/'
        }, {
            title: '第九章 常见问题',
            path: '/docs/09_FAQ/'
        }, {
            title: '附录1 人脸检测',
            path: '/docs/a1_face_detect/'
        }, {
            title: '附录2 流媒体服务器',
            path: '/docs/a2_media_server/'
        }]
    }
}

// npm run docs:dev
// npm run docs:build
