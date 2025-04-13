#pragma once
#include <opencv2/opencv.hpp>
#include <string>

using namespace cv;
using namespace std;

class VideoPack {
public:
  VideoPack();
  bool push(cv::Mat mat, int index);
  std::string addWav(const std::string &wav);
  cv::VideoWriter _writer;
  bool _inited = false;
  std::string _temp = "";
  std::string _final = "";
};
