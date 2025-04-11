#pragma once
#include <string>

class config {
private:
  config() {};

public:
  static config *get();
  bool valid();
  std::string groupId = "";
  std::string apiKey = "";
};
