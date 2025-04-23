#include "config.h"
#include "clog.h"

config *config::get() {
  static config conf;
  return &conf;
}

bool config::valid() {
  auto check = [](const std::string &key) {
    if (key.size() == 0) {
      PLOGE << key << " empty, check the config file";
    }
  };
  check(groupId);
  check(apiKey);
  check(lmUrl);
  check(lmApiKey);
  check(lmModel);
  check(lmPrompt);
  return groupId.size() > 0 and apiKey.size() > 0;
}
