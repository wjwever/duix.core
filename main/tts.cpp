#include "tts.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <random>

#include "audio.h"
#include "config.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using json = nlohmann::json;

// 将单个Hex字符转换为对应的数值
unsigned char hexCharToValue(char c) {
  c = toupper(c);
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return 10 + (c - 'A');
  throw runtime_error("Invalid hex character");
}

// 将两个Hex字符转换为一个字节
unsigned char hexPairToByte(char high, char low) {
  return (hexCharToValue(high) << 4) | hexCharToValue(low);
}

// 从Hex字符串转换为二进制数据
vector<unsigned char> hexStringToBytes(const string &hexStr) {
  // 移除所有空白字符
  string cleanedHex;
  copy_if(hexStr.begin(), hexStr.end(), back_inserter(cleanedHex),
          [](char c) { return !isspace(c); });

  // 检查长度是否为偶数
  if (cleanedHex.length() % 2 != 0) {
    throw runtime_error("Hex string must have even length");
  }

  vector<unsigned char> bytes;
  bytes.reserve(cleanedHex.length() / 2);

  for (size_t i = 0; i < cleanedHex.length(); i += 2) {
    bytes.push_back(hexPairToByte(cleanedHex[i], cleanedHex[i + 1]));
  }

  return bytes;
}

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

static std::string writeWav(const std::string &bytes) {
  std::error_code ec;
  std::filesystem::create_directory("audio", ec);

  const auto &uid = uuid();

  std::string mp3 = "./audio/" + uid + ".mp3";
  std::ofstream oss(mp3.c_str(), std::ios::binary);

  auto data = hexStringToBytes(bytes);
  oss.write(reinterpret_cast<const char *>(data.data()), data.size());
  std::string wav = "./audio/" + uid + ".wav";
  mp3ToWavSystem(mp3, wav);

  oss.close();
  return wav;
}

// 用于存储响应数据的结构体
struct MemoryStruct {
  char *memory;
  size_t size;
};

// 回调函数，处理接收到的数据
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
  if (!ptr) {
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  // std::cout << mem->memory << std::endl;

  // auto root = json::parse(mem->memory);
  // writeWav(root["data"]["audio"]);

  return realsize;
}

namespace tts {

std::string pack(const std::string &text) {
  auto root = json::parse(R"(
	                       {
		                     "model":"speech-02-hd",
		                     "text":"真正的危险不是计算机开始像人一样思考，而是人开始像计算机一样思考。计算机只是可以帮我们处理一些简单事务。",
		                     "stream":false,
		                     "language_boost":"auto",
		                     "output_format":"hex",
		                     "voice_setting":{
			                   "voice_id":"male-qn-qingse",
			                   "speed":1,
			                   "vol":1,
			                   "pitch":0,
			                   "emotion":"happy"
		                     },
		                     "audio_setting":{
			                   "sample_rate":16000,
			                   "bitrate":128000,
			                   "format":"mp3"
		                     }
	                       }
	                       )");

  if (text.size() > 0) {
    root["text"] = text;
  }
  return root.dump();
}

std::string tts(const std::string &text) {
  auto *config = config::get();
  CURL *curl;
  CURLcode res;
  struct MemoryStruct chunk;

  chunk.memory = (char *)malloc(1); // 初始化为空字符串
  chunk.size = 0;

  // in main
  // curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  struct curl_slist *headers = NULL;
  std::string wavPath = "";

  if (curl) {
    std::string url =
        "https://api.minimax.chat/v1/t2a_v2?GroupId=" + config->groupId;
    // 设置请求URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // 设置HTTP头
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string apiKey = "Authorization: Bearer " + config->apiKey;
    headers = curl_slist_append(headers, apiKey.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // 设置为POST请求
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    // 设置POST数据
    const std::string &data = pack(text);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

    // 设置回调函数处理响应
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // 执行请求
    res = curl_easy_perform(curl);

    // 检查错误
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    } else {
      // 打印响应数据
      // printf("%s\n", chunk.memory);
      auto root = json::parse(chunk.memory);
      wavPath = writeWav(root["data"]["audio"]);
    }

    // 清理
    free(chunk.memory);
    curl_easy_cleanup(curl);
  }

  curl_slist_free_all(headers);

  // in main
  // curl_global_cleanup();
  return wavPath;
}

} // namespace tts
