## conversation_system：会话管理系统


#### 功能介绍：

    针对谈话场景，完成多路视频与声音的采集，实时播放封装与存储。

#### 开发环境：

    Windows、Visual Studio、OpenCV、FFmpeg 

#### 文件介绍：

    queue.h、queue.cpp：队列函数的实现；

    ffmpeg_Capture.h、ffmpeg_Capture.cpp：ffmpeg采集网络摄像头；

    output_play.h、output_play.cpp：打开多路摄像头，合并在同一图片上显示，并能根据鼠标事件切换画面；

    ffmpeg_muxer.h、ffmpeg_muxer.cpp：ffmpeg采集音频，并与合并后的画面实时封装，保存为mp4文件；

    video_parameter.ini：更改参数的配置文件；
