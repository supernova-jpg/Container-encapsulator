目前该程序仍然存在以下问题：
- 1.程序的ffmpeg命令不能吃入分辨率、帧率、位深等关键信息，导致封装过程彻底失败，目前得到报错信息如下：
```
09:27:46 [INFO] FFmpeg environment configured successfully
09:29:38 [INFO] Added 1 files to processing list
09:29:50 [INFO] Created output folder: C:/Users/w60031491/Documents/ProMuxer_Output
09:29:50 [INFO] Starting batch processing...
09:29:50 [INFO] Starting to process 1 files...
09:29:50 [INFO] Output folder: C:/Users/w60031491/Documents/ProMuxer_Output
09:29:50 [INFO] Output format: mp4
09:29:50 [INFO] FFmpeg path: ffmpeg.exe
09:29:50 [INFO] Processing file 1/1: B00_10bit_HappyCrowd_1920x1080_30fps_420p10le_encoder_h265_FHD_10bit_B_9000000.265
09:29:50 [INFO] Starting FFmpeg: "ffmpeg.exe" -i \10.142.116.58\l60049343\jingfen\V900_xiaomi\20250707\1_bin\fhd_10bit\V900-1B-VBR\B00_10bit_HappyCrowd_1920x1080_30fps_420p10le_encoder_h265_FHD_10bit_B_9000000.265 -c copy -map 0 -f mp4 -movflags faststart C:\Users\w60031491\Documents\ProMuxer_Output\B00_10bit_HappyCrowd_1920x1080_30fps_420p10le_encoder_h265_FHD_10bit_B_9000000_muxed.mp4
09:29:50 [INFO] FFmpeg failed with exit code 1 Last output:
09:29:50 [INFO] ✗ Failed to process: B00_10bit_HappyCrowd_1920x1080_30fps_420p10le_encoder_h265_FHD_10bit_B_9000000.265 - FFmpeg failed with exit code 1 Last output:
09:29:50 [INFO] All files processed successfully!
09:29:50 [INFO] All files processed successfully!
09:33:15 [INFO] Removed file from list
09:33:17 [INFO] Added 1 files to processing list; 
```
你的修改，应当保证只要用户输入了正确的元数据，就一定能生成正确的ffmpeg命令进行封装。

- 2. 我在UI界面中要点击“Analyze Files”，程序才会开始尝试解析元数据，这个做法也有些欠妥。正确的做法是点击"Add Files"或者“Add Folder”,程序就应该立刻开始尝试解析分辨率、帧率、位深等元数据，并移除“Analyze Files”图标；
- 3. 如果Windows UI界面被设置为黑色背景，那么你Log Output当中显示的颜色应当是白色的，现在你的黑色文字让我根本就看不清日志的文字，反之亦然。
