#pragma once
#include <curl/curl.h>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include "util.h"

class Fetch {
public:
  // 响应结构体
  struct Response {
    int status_code;
    std::string status_text;
    std::string url;
    std::string body;
    std::map<std::string, std::string> headers;

    bool ok() const { return status_code >= 200 && status_code < 300; }
  };

  // 请求选项结构体
  struct RequestOptions {
    std::string method = "GET";
    std::map<std::string, std::string> headers;
    std::string body;
    bool follow_redirects = true;
    long timeout_ms = 0; // 0表示不超时
    std::function<void(char *, size_t)> onData;
  };

  // 初始化cURL
  // static void init() {
  //     curl_global_init(CURL_GLOBAL_ALL);
  // }

  // 清理cURL
  // static void cleanup() {
  //     curl_global_cleanup();
  // }

  // 执行HTTP请求
  static Response request(const std::string &url,
                          const RequestOptions &options);
  // 简化方法
  static Response get(const std::string &url,
                      const std::map<std::string, std::string> &headers = {});

  static Response post(const std::string &url, const std::string &body,
                       const std::map<std::string, std::string> &headers = {});

private:
  // 响应体写入回调
  static size_t writeCallback(void *contents, size_t size, size_t nmemb,
                              void *userp);
  // 响应头处理回调
  static size_t headerCallback(char *buffer, size_t size, size_t nitems,
                               void *userdata);
};
