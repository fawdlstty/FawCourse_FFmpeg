# 第七章 Filter 滤镜

在讲述这章前，我先提一个问题：如何给视频加水印？

Windows客户端开发的小伙伴第一个想到的是GDI+吧？视频读入GDI+，然后每帧画一个水印，再写入新的视频中；Qt也一样，其他绘图库同理。但这会有一个潜在的问题，使用GDI+的不跨平台、使用Qt和其他绘图库或多或少附带一堆库程序，并且效率会较低，因为始终会有数据在ffmpeg与绘图库间传输。

FFmpeg解决这些问题的方式是：Filter（滤镜），也可以理解为FFmpeg官方绘图库，转为视频处理而生，相比其他绘图库能极大提高绘制效率。

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

参考：[ffmpeg-filters](https://ffmpeg.org/ffmpeg-filters.html)

[返回首页](../README.md) | [上一章 FFmpeg与其他库的交互](./06_interaction.md) | 下一章

## 许可

[![test](https://i.creativecommons.org/l/by-nc-nd/4.0/80x15.png)](http://creativecommons.org/licenses/by-nc-nd/4.0/)

本教程采用[知识共享署名-非商业性使用-禁止演绎 4.0 国际许可协议](http://creativecommons.org/licenses/by-nc-nd/4.0/)许可。
