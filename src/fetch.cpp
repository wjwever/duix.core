#include "fetch.h"
#include <curl/curl.h>

Fetch::Response Fetch::request(const std::string &url,
                               const Fetch::RequestOptions &options) {
  Response response;
  response.url = url;
  CURL *curl = curl_easy_init();
  if (!curl) {
    // throw std::runtime_error("Failed to initialize cURL");
    response.status_code = -1;
    return response;
  }

  // 设置cURL选项
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, options.method.c_str());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,
                   options.follow_redirects ? 1L : 0L);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, options.timeout_ms);

  // 设置请求头
  struct curl_slist *headers = nullptr;
  for (const auto &[key, value] : options.headers) {
    std::string header = key + ": " + value;
    headers = curl_slist_append(headers, header.c_str());
  }
  if (headers) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }

  // 设置请求体
  if (!options.body.empty() &&
      (options.method == "POST" || options.method == "PUT" ||
       options.method == "PATCH")) {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, options.body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, options.body.size());
  }

  // 响应处理回调
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &options);

  // 头处理回调
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);

  // 执行请求
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    // throw std::runtime_error("cURL request failed: " +
    //                          std::string(curl_easy_strerror(res)));
    response.status_code = -1;
    return response;
  }

  // 获取状态码
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);

  // 清理
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

  return response;
}

Fetch::Response Fetch::get(const std::string &url,
                           const std::map<std::string, std::string> &headers) {
  Fetch::RequestOptions options;
  options.method = "GET";
  options.headers = headers;
  return Fetch::request(url, options);
}

Fetch::Response Fetch::post(const std::string &url, const std::string &body,
                            const std::map<std::string, std::string> &headers) {
  Fetch::RequestOptions options;
  options.method = "POST";
  options.headers = headers;
  options.body = body;
  return Fetch::request(url, options);
}

size_t Fetch::writeCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  size_t realsize = size * nmemb;
  auto func = static_cast<Fetch::RequestOptions *>(userp)->onData;
  if (func) {
    func(static_cast<char *>(contents), realsize);
  }

  // static_cast<std::string *>(userp)->append(static_cast<char *>(contents),
  //                                           realsize);
  return realsize;
}

size_t Fetch::headerCallback(char *buffer, size_t size, size_t nitems,
                             void *userdata) {
  size_t realsize = size * nitems;
  std::string header(buffer, realsize);

  auto colon_pos = header.find(':');
  if (colon_pos != std::string::npos) {
    std::string key = header.substr(0, colon_pos);
    std::string value = header.substr(colon_pos + 1);

    // 去除首尾空白字符
    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t") + 1);
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);

    auto *headers = static_cast<std::map<std::string, std::string> *>(userdata);
    (*headers)[key] = value;
  }

  return realsize;
}
