## 软件问题单汇总

### 需求一：ffmpeg检测逻辑不健壮
我在当前电脑环境中运行 ffmpeg -version的命令后，已经可以显示出ffmpeg的信息，但是软件仍然提示"FFmpeg not found in PATH"
我希望您进一步改善对ffmpeg环境变量的支持，在程序启动时就运行 ffmpeg -version命令检测ffmpeg是否已被添加至环境变量中。保证我只要添加了ffmpeg环境变量，软件就一定能正确的检测到ffmpeg的相关信息，类似于"FFprobe found in PATH - Version: 7.1.1-full_build-www.gyan.dev, Release Years: 2007-2025"这样的信息应该在软件中被明确显示


### 需求二：软件不支持一键全选
有时候软件可能无法正确的解析位深、色彩空间等码流元数据，但考虑到一个文件夹下面的码流数据很可能都是一样的，我期望在一个文件选好分辨率、位深等信息后，可以把这个设置一键应用到全部的码流文件中。

### 需求三：软件新增.bin重建为.yuv功能（重大改进）
考虑到现在软件已经有了比较完善的手动选择分辨率、位深、帧率编解码协议等元数据的功能，我希望您能进一步开发出.bin重建为.yuv的功能
- YUV**必须**遵循如下命名规范：
并确保转换后的YUV文件名符合以下命名规范：
- (序列代号)_(比特位深)bit_(场景描述名)_(分辨率)_(帧率)fps_420p/yuv420p10le.yuv
示例合法文件名：
- 05_8bit_IndoorTexture_3840x2160_30fps_420p.yuv
- A00_10bit_Sanitationer_1920x1080_30fps_yuv420p10le.yuv
