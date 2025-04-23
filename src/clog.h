/*************************************************************************
    > File Name: plog.h
    > Author: 1216451203@qq.com
    > Mail: 1216451203@qq.com
    > Created Time: 2025年03月11日 星期二 23时16分31秒
 ************************************************************************/
#pragma once
#include <ctime>
#include <sstream>

enum class PLogLevel { INFO, DEBUG, WARNING, ERROR };

class CLogger {
public:
  CLogger(PLogLevel lever, const char *file, int line);
  ~CLogger();
  std::string toString() const;
  template <typename T> CLogger &operator<<(const T &value) {
    log_stream_ << value;
    return *this;
  }
  CLogger &ref();

  PLogLevel lever() const;

private:
  int line_;
  PLogLevel level_;
  std::string file_;
  std::string level_str_;
  std::stringstream log_stream_;
};

class LogDump {
public:
  LogDump();
  void operator+=(const CLogger &logger);
};

#define PLOGI LogDump() += CLogger(PLogLevel::INFO, __FILE__, __LINE__).ref()
#define PLOGD LogDump() += CLogger(PLogLevel::DEBUG, __FILE__, __LINE__).ref()
#define PLOGE LogDump() += CLogger(PLogLevel::ERROR, __FILE__, __LINE__).ref()
