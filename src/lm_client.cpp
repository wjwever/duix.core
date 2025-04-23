#include "lm_client.h"
#include "clog.h"
#include "config.h"
#include "fetch.h"
#include "util.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using std::string;

void LmClient::request(const std::string &query) {
  if (chat.load()) {
    return;
  }
  chat = true;
  auto *config = config::get();
  json messages = {{{"role", "system"}, {"content", config->lmPrompt}}};
  for (int i = 0; i < _history.size(); ++i) {
    std::string role = i % 2 == 0 ? "user" : "system";
    messages.push_back({{"role", role}, {"content", _history[i]}});
  }
  messages.push_back({{"role", "user"}, {"content", query}});
  nlohmann::json root = {
      {"model", config->lmModel}, {"messages", messages}, {"stream", true}};

  Fetch::RequestOptions options;
  options.method = "POST";
  options.headers = {{"Content-Type", "application/json"},
                     {"Authorization", "Bearer " + config->lmApiKey},
                     {"Accept", "text/event-stream"}};
  options.body = root.dump();

  std::string buffer;
  std::string response;
  std::string tmpResp;
  bool done = false;

  options.onData = [&](char *data, size_t len) {
    // buffer += chunk;
    if (done) {
      return;
    }
    buffer.append(data, len);

    // 处理SSE (Server-Sent Events)格式
    size_t pos = 0;
    while ((pos = buffer.find("\n\n")) != std::string::npos) {
      std::string event = buffer.substr(0, pos);
      buffer.erase(0, pos + 2);

      if (event.empty())
        continue;

      // 解析事件数据
      if (event.substr(0, 6) == "data: ") {
        std::string data = event.substr(6);
        PLOGI << "data:" << data;

        if (data == "[DONE]") {
          done = true;
          break;
        }

        try {
          auto json = nlohmann::json::parse(data);
          response += json["choices"][0]["delta"]["content"];
          tmpResp += json["choices"][0]["delta"]["content"];
          auto arr = splitMixedSentences(tmpResp, false);
          onSubText(arr);
        } catch (const std::exception &e) {
          PLOGE << "JSON parse error: " << data << " " << e.what();
          done = true; 
          break;
        }
      }
    }

    if (done) {
        /*
        auto json = nlohmann::json::parse(buffer);
        response += json["choices"][0]["content"];
        tmpResp += json["choices"][0]["content"];
        */
        auto arr = splitMixedSentences(tmpResp, true);
        arr.push_back("");
        onSubText(arr);
    }
  };

  Fetch::request(config->lmUrl + "/chat/completions", options);
  _history.push_back(query);
  _history.push_back(response);
  chat = false;
}
