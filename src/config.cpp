#include "config.h"

config *config::get() {
  static config conf;
  return &conf;
}

bool config::valid() { return groupId.size() > 0 and apiKey.size() > 0; }
