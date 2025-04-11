#include "video.h"
#include <filesystem>
#include <random>
using namespace cv;
using namespace std;

static std::string uuid() {
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<uint64_t> dis;

  uint64_t part1 = dis(gen);
  uint64_t part2 = dis(gen);

  std::ostringstream oss;
  oss << std::hex << part1 << part2;

  std::string uuidStr = oss.str();
  return uuidStr.substr(0, 32); // 截取32位作为简化的UUID
}

VideoPack::VideoPack() {
  std::error_code ec;
  std::filesystem::create_directory("video", ec);
  const std::string &uid = uuid();
  _temp = "video/temp_" + uid + ".mp4";
  _final = "video/" + uid + ".mp4";
}

std::string VideoPack::addWav(const std::string &wav) {
  _writer.release();
  // 2. 使用FFmpeg合并视频和音频
  string command = "ffmpeg -y -i " + _temp + " -i " + wav +
                   " -c:v copy -c:a aac -strict experimental -map 0:v:0 -map "
                   "1:a:0 -shortest " +
                   _final;

  cout << "Executing: " << command << endl;
  int result = system(command.c_str());

  if (result != 0) {
    cerr << "Error merging audio and video!" << endl;
    return "";
  }
  return _final;
}

bool VideoPack::push(cv::Mat mat, int index) {
  // 获取第一帧的尺寸作为视频尺寸
  Size frameSize = mat.size();

  if (index == 0) {
    // 尝试几种可能的编码器
    vector<string> codecsToTry = {"avc1", "mp4v", "h264", "x264"};
    for (const auto &c : codecsToTry) {
      int fourcc = VideoWriter::fourcc(c[0], c[1], c[2], c[3]);
      _writer.open(_temp, fourcc, 25.0, frameSize, true);

      if (_writer.isOpened()) {
        _inited = true;
        cout << "Using codec: " << c << endl;
        break;
      }
    }

    if (!_inited) {
      cerr << "Error: Could not initialize video writer with any supported "
              "codec!"
           << endl;
      cerr << "Tried codecs: ";
      for (const auto &c : codecsToTry)
        cerr << c << " ";
      cerr << endl;
      return false;
    }
  }
  _writer.write(mat);
  return true;

#if 0
  // 写入所有帧
  for (const auto &frame : frames) {
    // 检查帧尺寸是否一致
    if (frame.size() != frameSize) {
      cerr << "Warning: Frame size mismatch! Expected " << frameSize << ", got "
           << frame.size() << ". Resizing..." << endl;

      Mat resizedFrame;
      resize(frame, resizedFrame, frameSize);
      writer.write(resizedFrame);
    } else {
      writer.write(frame);
    }
  }

  // 释放资源
  // writer.release();
  // cout << "Video saved successfully to: " << outputPath << endl;
#endif
}
