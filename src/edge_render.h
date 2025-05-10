/*************************************************************************
    > File Name: edge_render.h
    > Author: 1216451203@qq.com
    > Mail: 1216451203@qq.com
    > Created Time: 2025年03月11日 星期二 22时46分16秒
 ************************************************************************/
#pragma once
#include "block_queue.h"
#include "clog.h"
#include "video.h"
#include <atomic>
#include <digit/GDigit.h>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <model_info.h>
#include <string>

typedef std::function<void(std::vector<uint8_t> &data)> ImgHdl;
typedef std::function<void(const std::string &msg)> MsgHdl;

// 线程安全的RGBA帧队列
template <class T> class SafeQueue {
private:
  std::queue<T> queue;
  mutable std::mutex mutex;
  std::condition_variable cond;
  std::atomic<bool> run;

public:
  void stop() {
    run = false;
  }
  SafeQueue() {
    run = true;
  }
  ~SafeQueue() {
    stop();
  }
  std::string name = "default";
  size_t max_size = 5; // 限制队列大小防止内存溢出
  void push(T &wav) {
    std::unique_lock<std::mutex> lock(mutex);
    while(run.load()) {
        if (cond.wait_for(lock, std::chrono::seconds(1),
                      [this] { return queue.size() < max_size; })) {
        queue.push(std::move(wav));
        break;
        } 
    }
  }

  bool try_pop(T &frame) {
    std::lock_guard<std::mutex> lock(mutex);
    if (queue.empty()) {
      return false;
    }
    frame = std::move(queue.front());
    queue.pop();
    cond.notify_one();
    return true;
  }
};

class EdgeRender {
public:
  EdgeRender();
  ~EdgeRender() {
    _done.store(true);
    _ttsTasks.stop();
    _wavs.stop();
    _frames.stop();
    _thRender.join();
    _thSender.join();
    _thWav.join();
    PLOGD << "EdgeRender exit";
  }
  int load(const std::string &role);
  void setImgHdl(ImgHdl handler);
  void setMsgHdl(MsgHdl handler);
  int checkModel(const std::string &role);

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

  void startRender();
  SafeQueue<std::future<std::string>> _ttsTasks;
  SafeQueue<std::string> _wavs;
  SafeQueue<std::shared_ptr<std::vector<uint8_t>>> _frames;
  std::thread _thRender;
  std::thread _thSender;
  std::thread _thWav;

  std::string _asr;
  std::mutex _mx_asr;
  void setAsr(const std::string& asr) {
    std::lock_guard<std::mutex> lk(_mx_asr);
    _asr = asr;
  }

  ImgHdl _imgHdl;
  MsgHdl _msgHdl;
};
