# FawCourse_FFmpeg 示例代码

## [读取摄像头并展示-SDL2.cpp](./读取摄像头并展示-SDL2.cpp)

程序结构：

```cpp
#include <头文件>

int main (int argc, char* argv[]) {
    // 初始化
    // 打开输入流
    while (_run) {
        // 从摄像头输入流中读一个AVPacket
        // 将AVPacket解码为AVFrame
        // 转换AVFrame的像素格式
        // 展示图片
    }
    // 关闭输入流
    return 0;
}
```

## [读取摄像头并展示-Win32.cpp](./读取摄像头并展示-Win32.cpp)

程序结构：

```cpp
#include <头文件>

// 消息循环
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) {
    if (msg == WM_PAINT) {
        // 从全局变量读图片
        // 绘制
    }
}

int WINAPI _tWinMain (HINSTANCE, HINSTANCE, LPTSTR, int) {
    // 初始化
    // 打开输入流
    new_thread {
        while (_run) {
            // 图像处理
            // 从摄像头输入流中读一个AVPacket
            // 将AVPacket解码为AVFrame
            // 转换AVFrame的像素格式
            // 将图片放入全局变量
        }
    }
    // 进入消息循环
    // 关闭输入流
    return 0;
}
```