/*************************************************************************
    > File Name: plog.cpp
    > Author: wangjiawei
    > Mail: 1216451203@qq.com
    > Created Time: 2025年03月11日 星期二 23时23分33秒
 ************************************************************************/
#include <clog.h>
#include <iostream>
#include <mutex>
CLogger::CLogger(PLogLevel level, const char *file, int line) {
  line_ = line;
  std::string filePath(file);
  file_ = filePath.substr(filePath.find_last_of("/\\") + 1);
  level_ = level;

  switch (level_) {
  case PLogLevel::INFO:
    level_str_ = "[INFO]";
    break;
  case PLogLevel::DEBUG:
    level_str_ = "[DEBUG]";
    break;
  case PLogLevel::WARNING:
    level_str_ = "[WARNING]";
    break;
  case PLogLevel::ERROR:
    level_str_ = "[ERROR]";
    break;
  }
  log_stream_ << level_str_ << " [" << file_ << ":" << line_ << "]";
}

CLogger::~CLogger() {}

std::string CLogger::toString() const { return log_stream_.str(); }

CLogger &CLogger::ref() { return *this; }

PLogLevel CLogger::lever() const { return level_; }

LogDump::LogDump() {}

void LogDump::operator+=(const CLogger &logger) {
  static std::mutex mutex;
  std::lock_guard<std::mutex> lock(mutex);
  std::cout << logger.toString() << std::endl;
}
