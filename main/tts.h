#pragma once
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>

namespace tts {
std::string pack(const std::string &text, const std::string &voice);
std::string tts(const std::string &text, const std::string &voice);
} // namespace tts
