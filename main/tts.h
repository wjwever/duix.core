#pragma once
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>

namespace tts {
std::string pack(const std::string &text);
std::string tts(const std::string &text);
} // namespace tts
