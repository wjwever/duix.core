#pragma once
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <vector>

#include <chrono>
#include <iomanip>
#include <iostream>
#include "clog.h"

std::u32string utf8ToUtf32(const std::string &utf8_str);
std::string utf32ToUtf8(const std::u32string &utf32_str);
std::vector<std::string> splitMixedSentences(std::string &text, bool last);

inline std::string getCurrentTime() {
  // 获取当前时间点
  auto now = std::chrono::system_clock::now();
  // 转换为time_t，以便使用put_time
  auto now_c = std::chrono::system_clock::to_time_t(now);
  // 转换为time_point，以便获取毫秒部分
  auto duration = now.time_since_epoch();
  auto millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

  // 使用stringstream来构建完整的日期时间字符串
  std::stringstream ss;
  ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
  ss << "." << std::setw(3) << std::setfill('0') << (millis % 1000); // 毫秒部分

  return ss.str();
}

class Timer {
public:
  Timer(const std::string &name = "")
      : name(name), start(std::chrono::high_resolution_clock::now()) {}

  ~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    PLOGD << "cost: " << name << " " << duration.count() << " 毫秒";
  }

private:
  std::string name;
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

// 回调函数，用于接收HTTP响应数据
static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

inline std::string getPublicIP() {
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  curl = curl_easy_init();
  if (curl) {
    // 使用ipify.org服务获取IP
    curl_easy_setopt(curl, CURLOPT_URL, "https://ipinfo.io/ip");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
      return "Error: " + std::to_string(res);
    }

    return readBuffer;
  }
  return "Failed to initialize CURL";
}

/*
int main() {
    std::string publicIP = getPublicIP();
    std::cout << "Public IP: " << publicIP << std::endl;
    return 0;
}
*/
