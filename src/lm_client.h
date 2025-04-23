#pragma once
#include "functional"
#include <atomic>
#include <string>
#include <vector>
#include <future>

/*
struct TTSTask {
  std::string text = 0;
  std::string wav = 0;
  std::future<std::string> fut;
  bool last = true;
};
*/

class LmClient {
public:
  void request(const std::string &query);
  std::vector<std::string> _history;
  // std::vector<std::string> splitSentence(std::string &text, bool last);
  std::function<void(const std::vector<std::string> &)> onSubText;
  std::atomic<bool> chat = false;
};
