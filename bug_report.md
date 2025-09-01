## 软件问题单汇总

### 需求一：ffmpeg检测逻辑不健壮
我在当前电脑环境中运行 ffmpeg -version的命令后，已经可以显示出ffmpeg的信息，但是软件仍然提示"FFmpeg not found in PATH"
我希望您进一步改善对ffmpeg环境变量的支持，在程序启动时就运行 ffmpeg -version命令检测ffmpeg是否已被添加至环境变量中。保证我只要添加了ffmpeg环境变量，软件就一定能正确的检测到ffmpeg的相关信息，类似于"FFprobe found in PATH - Version: 7.1.1-full_build-www.gyan.dev, Release Years: 2007-2025"这样的信息应该在软件中被明确显示


### 需求二：封装码流不符合预期
命令封装出的码流只能播放前1秒，之后的视频信息都被快速跳过去了，而没有保存全部的码流信息
我们使用的命令类似于如下：Starting FFmpeg: "ffmpeg.exe" -fflags +genpts -framerate 30 -i \\10.142.116.58\l60049343\jingfen\V900_xiaomi\20250707\1_bin\fhd_10bit\xiaomi15-1B-VBR\B12_10bit_OutdoorBuliding_1920x1080_30fps_p010_HEVC_abr10000k.bin -c:v copy -f mp4 -movflags faststart C:\Users\w60031491\Documents\ProMuxer_Output\B12_10bit_OutdoorBuliding_1920x1080_30fps_p010_HEVC_abr10000k_muxed.mp4
右键查看.mp4的详细信息，发现“视频时长”只有1秒钟，“帧速率”高达224.96帧/秒，完全不符合预期，即使我们加了-framerate 30参数。
请你根据上述命令排查问题，修改代码完成我的需求
