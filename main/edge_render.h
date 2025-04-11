/*************************************************************************
    > File Name: edge_render.h
    > Author: wangjiawei
    > Mail: 1216451203@qq.com
    > Created Time: 2025年03月11日 星期二 22时46分16秒
 ************************************************************************/
#pragma once
#include "block_queue.h"
#include "video.h"
#include <atomic>
#include <digit/GDigit.h>
#include <map>
#include <memory>
#include <model_info.h>
#include <string>

class EdgeRender {
public:
  EdgeRender();
  int load(const std::string &baseDir, const std::string &modelDir);

  std::string render(const std::string &wav);
  void getMsg(std::string &msg);
  bool done() { return _done.load(); }

  std::map<std::string, std::string> _baseMD5Map;
  std::map<std::string, std::string> _modelMD5Map;

  ModelInfo _modelInfo;
  std::unique_ptr<GDigit> _digit;
  VideoPack _videoPack;
  BlockQueue<std::string> _queue;
  std::atomic<bool> _done;
};
