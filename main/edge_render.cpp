/*************************************************************************

    > File Name: edge_render.cpp
    > Author: wangjiawei
    > Mail: 1216451203@qq.com
    > Created Time: 2025年03月11日 星期二 22时50分37秒
 ************************************************************************/

#include "aesmain.h"
#include "block_queue.h"
#include "config.h"
#include <clog.h>
#include <edge_render.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <wav_reader.h>

namespace fs = std::filesystem;
using json = nlohmann::json;

EdgeRender::EdgeRender() {
  _baseMD5Map = {{"alpha_model.b", "ab"},
                 {"alpha_model.p", "ap"},
                 {"cacert.p", "cp"},
                 {"weight_168u.b", "wb"},
                 {"wenet.o", "wo"}};
  _modelMD5Map = {{"dh_model.b", "db"},
                  {"dh_model.p", "dp"},
                  {"bbox.j", "bj"},
                  {"config.j", "cj"},
                  {"weight_168u.b", "wb"}

  };
  _done.store(false);
}

int EdgeRender::checkModel(const std::string &role) {
  static std::mutex mx;
  std::lock_guard<std::mutex> lock(mx);

  fs::path model = "gj_dh_res";
  std::string url = "https://cdn.guiji.ai/duix/location/gj_dh_res.zip";
  if (fs::exists(model) == false) {
    std::string cmd = "wget " + url + ";unzip gj_dh_res.zip";
    std::system(cmd.c_str());
    if (fs::exists(model) == false) {
      PLOGD << "failed to run command:" << cmd;
      return -1;
    }
  }

  auto conf = config::get();
  if (conf->roles.count(role) == 0) {
    PLOGD << "Not support role: " << role << " use default role";
    url = conf->roles["XiaoXuan"];
  } else {
    url = conf->roles[role];
  }

  fs::path roleDir = "roles/" + role;
  if (fs::exists(roleDir) == false) {
    std::string zipName = fs::path(url).filename();
    std::string roleName = zipName.substr(0, zipName.length() - 4);
    std::string cmd = "mkdir -p roles";
    std::system(cmd.c_str());
    cmd = "wget " + url + ";" + "unzip " + zipName + ";" + "mv " + roleName +
          " " + roleDir.string();
    std::system(cmd.c_str());
    if (fs::exists(roleDir) == false) {
      PLOGD << "failed to run:" << cmd;
      return -2;
    }
  }

  return 0;
}

int EdgeRender::load(const std::string &role) {
  std::string baseDir = "gj_dh_res";
  std::string modelDir = "roles/" + role;
  if (checkModel(role) != 0) {
    return -1;
  };
  for (const auto &p : _baseMD5Map) {
    auto &key = p.first;
    auto &value = p.second;
    fs::path file = baseDir + "/" + value;
    if (fs::exists(file) == false) {
      fs::path newFile = baseDir + "/" + key;
      PLOGD << "convert " << newFile.string() << " to " << file.string();
      if (!fs::exists(newFile)) {
        PLOGD << "cant find " << newFile.string();
        return -1;
      }
      int ret = mainenc(0, const_cast<char *>(newFile.string().c_str()),
                        const_cast<char *>(file.string().c_str()));
      PLOGD << "convert result:" << ret;
    }
  }
  for (const auto &p : _modelMD5Map) {
    auto &key = p.first;
    auto &value = p.second;
    fs::path file = modelDir + "/" + value;
    if (fs::exists(file) == false) {
      fs::path newFile = modelDir + "/" + key;
      PLOGD << "convert " << newFile.string() << " to " << file.string();
      if (!fs::exists(newFile)) {
        PLOGD << "cant find " << newFile.string();
        return -1;
      }
      int ret = mainenc(0, const_cast<char *>(newFile.string().c_str()),
                        const_cast<char *>(file.string().c_str()));
      PLOGD << "convert result:" << ret;
    }
  }
  std::ifstream bbox(modelDir + "/" + _modelMD5Map["bbox.j"]);
  json boxJson = json::parse(bbox);

  std::ifstream config(modelDir + "/" + _modelMD5Map["config.j"]);
  json configJson = json::parse(config);

  _modelInfo._hasMask = configJson.value("need_png", 0) == 0 and
                        fs::exists(modelDir + "/raw_jpgs") and
                        fs::exists(modelDir + "/raw_sg") and
                        fs::exists(modelDir + "/pha");

  PLOGD << "hasMask:" << _modelInfo._hasMask;
  for (int i = 1;; ++i) {
    auto rawPath = modelDir + "/raw_jpgs/" + std::to_string(i) + ".sij"; //
    auto maskPath = modelDir + "/pha/" + std::to_string(i) + ".sij";
    auto sgPath = modelDir + "/raw_sg/" + std::to_string(i) + ".sij"; // front
    if (fs::exists(rawPath) == false) {
      break;
    }
    Frame frame;
    frame.index = i;
    frame._rawPath = rawPath;
    if (_modelInfo._hasMask and fs::exists(maskPath)) {
      frame._maskPath = maskPath;
    }
    if (_modelInfo._hasMask and fs::exists(sgPath)) {
      frame._sgPath = sgPath;
    }
    if (boxJson.count(std::to_string(i))) {
      frame.rect[0] = boxJson[std::to_string(i)][0];
      frame.rect[1] = boxJson[std::to_string(i)][2];
      frame.rect[2] = boxJson[std::to_string(i)][1];
      frame.rect[3] = boxJson[std::to_string(i)][3];
    }
    // PLOGD << frame.toString();
    _modelInfo._frames.push_back(frame);
  }
  if (_modelInfo._frames.size() == 0) {
    return -2;
  }

  _modelInfo._width = configJson.value("width", 0);
  _modelInfo._height = configJson.value("height", 0);

  //  TODO 处理动作

  json ncnnConfig;
  ncnnConfig["action"] = 1;
  ncnnConfig["videowidth"] = _modelInfo._width;
  ncnnConfig["videoheight"] = _modelInfo._height;
  ncnnConfig["timeoutms"] = 5000;
  ncnnConfig["wenetfn"] = baseDir + "/wo";
  if (fs::exists(modelDir + "/wb")) {
    PLOGD << "使用模型自带的weight:" << modelDir + "/wb";
    ncnnConfig["unetmsk"] = modelDir + "/wb";
  } else {
    ncnnConfig["unetmsk"] = baseDir + "/wb";
  }
  ncnnConfig["cacertfn"] = baseDir + "/cp";
  ncnnConfig["alphabin"] = baseDir + "/ab";
  ncnnConfig["alphaparam"] = baseDir + "/ap";

  ncnnConfig["unetbin"] = modelDir + "/db";
  ncnnConfig["unetparam"] = modelDir + "/dp";
  PLOGD << "ncnnConfig:" << ncnnConfig.dump();

  _modelInfo._ncnnConfig = ncnnConfig.dump();

  MessageCb *cb = nullptr;
  _digit = std::make_unique<GDigit>(_modelInfo._width, _modelInfo._height, cb);

  PLOGD << "digit config:" << _digit->config(_modelInfo._ncnnConfig.c_str());
  _digit->start();
  PLOGD << "width:" << _modelInfo._width << " height:" << _modelInfo._height;
  PLOGD << "模型初始化完成";

  return 0;
}

std::string EdgeRender::render(const std::string &input) {
  _done.store(false);

  const auto &wav = fixWav(input);

  int all_buf = _digit->newwav(wav.c_str(), "");

  double wavDuration = durationMs(wav);
  PLOGD << "buf_len:" << all_buf << " wav_len:" << wavDuration;

  cv::Mat mat = cv::Mat(_modelInfo._height, _modelInfo._width, CV_8UC3);
  cv::Mat mskmat = cv::Mat(_modelInfo._height, _modelInfo._width, CV_8UC3);
  for (int i = 0; i * 40 < wavDuration; ++i) {
    Frame frame = _modelInfo._frames[i % _modelInfo._frames.size()];

    if (_modelInfo._hasMask) {
      _digit->mskrstbuf(i, frame._rawPath.c_str(), frame.rect,
                        frame._maskPath.c_str(), frame._sgPath.c_str(),
                        reinterpret_cast<char *>(mat.data),
                        reinterpret_cast<char *>(mskmat.data),
                        _modelInfo._width * _modelInfo._height * 3);

      // _digit->drawmskbuf(frame._rawPath.c_str(), frame._maskPath.c_str(),
      //                    reinterpret_cast<char *>(mat.data),
      //                    reinterpret_cast<char *>(mskmat.data),
      //                    _modelInfo._height * _modelInfo._width * 3);

    } else {
      _digit->onerstbuf(i, frame._rawPath.c_str(), frame.rect,
                        reinterpret_cast<char *>(mat.data),
                        _modelInfo._height * _modelInfo._width * 3);
    }

#if 0
    cv::Point topLeft(frame.rect[0], frame.rect[1]);
    cv::Point bottomRight(frame.rect[2], frame.rect[3]);

    cv::Scalar color(255, 0, 0);
    int thickness = 3;
    // cv::rectangle(mat, topLeft, bottomRight, cv::Scalar(0, 255, 0), 3);

    char buffer[50] = {0};
    snprintf(buffer, sizeof(buffer), "out/%03d.png", i);

    if (cv::imwrite(buffer, mat) == false) {
      PLOGD << "Error saving image.";
    } else {
      PLOGD << "saving image " << buffer;
    }
#endif
    _videoPack.push(mat, i);
    double ratio = 4000 * i / wavDuration;
    json root;
    root["progress"] = std::to_string(ratio) + "%";
    _queue.push(root.dump() + "\n");
  }

  json root;
  root["video"] = _videoPack.addWav(wav);
  _queue.push(root.dump() + "\n");
  _done.store(true);
  return root["video"];
}

void EdgeRender::getMsg(std::string &msg) {
  msg = "";
  _queue.pop(msg);
}
