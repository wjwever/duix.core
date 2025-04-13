/*************************************************************************
    > File Name: main.cpp
    > Author: wangjiawei
    > Mail: 1216451203@qq.com
    > Created Time: 2025年03月12日 星期三 20时59分11秒
 ************************************************************************/

#include "clog.h"
#include "config.h"
#include "httplib.h"
#include "tts.h"
#include "util.h"
#include <edge_render.h>
#include <getopt.hpp>
#include <memory>
#include <string>
using namespace std;
using json = nlohmann::json;

int main() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  auto *config = config::get();
  std::string conf = getarg("conf/conf.json", "-c", "--conf");
  std::ifstream stream(conf);
  if (stream.is_open()) {
    auto root = json::parse(stream);
    if (root.count("minimax")) {
      config->groupId = root["minimax"].value("groupId", "");
      config->apiKey = root["minimax"].value("apiKey", "");
    }
  }

  if (config->valid() == false) {
    PLOGE << "config invalid:" << conf;
    return 0;
  }

  std::string IP = getPublicIP();
  PLOGD << "PublicIP:" << IP;
  httplib::Server svr;

  std::string cmd = "mkdir -p video";
  std::system(cmd.c_str());
  svr.set_mount_point("/video", "./video");

  // 处理 OPTIONS 预检请求
  svr.Options("/task_sse",
              [](const httplib::Request &req, httplib::Response &res) {
                res.set_header("Access-Control-Allow-Origin", "*");
                res.set_header("Access-Control-Allow-Headers", "Content-Type");
                res.status = 200; // No Content
              });

  svr.Post(
      "/task_sse", [&](const httplib::Request &req, httplib::Response &res) {
        // parse request
        PLOGD << "REQ:" << req.body;
        auto root = json::parse(req.body);
        std::string text = root.value("text", "请问有什么可以帮你的吗");
        std::string role = root.value("role", "siyao");
        std::string voice = root.value("voice", "male-qn-qingse");

        std::string baseDir = "gj_dh_res";
        auto wav = tts::tts(text, voice);

        auto render = std::make_shared<EdgeRender>();
        render->load(role);

        //  res.set_header("Content-Type", "text/event-stream");
        //  res.set_header("Cache-Control", "no-cache");
        //  res.set_header("Connection", "keep-alive");
        res.set_header("Access-Control-Allow-Origin", "*");
        //  res.set_header("Cache-Control", "no-cache");  // 禁用缓存
        //  res.set_header("Connection", "keep-alive");   // 保持长连接

        // 保持连接打开，手动控制响应
        res.set_chunked_content_provider(
            "text/event-stream",
            [render, &req, &IP](size_t offset, httplib::DataSink &sink) {
              std::string data;
              while (render and render->done() == false) {
                render->getMsg(data);
                try {
                  auto root = json::parse(data);
                  if (root.contains("video")) {
                    std::string newPath = "http://" + IP + ":8080/" +
                                          root.value("video", "video/tmp.mp4");
                    root["video"] = newPath;
                    data = root.dump();
                  }
                } catch (...) {
                  // data = "http://" + IP + ":8080/video/tmp.mp4";
                }
                if (data.size() > 0) {
                  PLOGD << data;
                  sink.write(data.data(), data.size());
                }
              }
              sink.done();
              return true;
            });
        std::thread th(&EdgeRender::render, render.get(), wav);
        th.detach();
      });

  PLOGD << "serve at http://" << IP << ":8080";
  svr.listen("0.0.0.0", 8080);
  curl_global_cleanup();
  return 0;
}
