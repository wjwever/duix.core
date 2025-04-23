/*************************************************************************
    > File Name: plog.cpp
    > Author: 1216451203@qq.com
    > Mail: 1216451203@qq.com
    > Created Time: 2025年03月11日 星期二 23时23分33秒
 ************************************************************************/
/*
#include <chrono>
#include <iomanip>
*/
#include <clog.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <util.h>

#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */

CLogger::CLogger(PLogLevel level, const char *file, int line) {
  line_ = line;
  std::string filePath(file);
  file_ = filePath.substr(filePath.find_last_of("/\\") + 1);
  level_ = level;

  std::string color = "";

  switch (level_) {
  case PLogLevel::INFO:
    level_str_ = "INFO";
    color = GREEN;
    break;
  case PLogLevel::DEBUG:
    level_str_ = "DEBUG";
    color = MAGENTA;
    break;
  case PLogLevel::WARNING:
    level_str_ = "WARNING";
    color = RED;
    break;
  case PLogLevel::ERROR:
    level_str_ = "ERROR";
    color = RED;
    break;
  }
  /*
  auto now = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;
  auto timer = std::chrono::system_clock::to_time_t(now);
  std::tm bt = *std::localtime(&timer);
  */

  log_stream_ << color.c_str() << "[" << level_str_ << " " << file_ << ":"
              << line_ << " " << std::this_thread::get_id() << " "
              << getCurrentTime() << "]\t" ;
}

CLogger::~CLogger() {}

std::string CLogger::toString() const { return log_stream_.str(); }

CLogger &CLogger::ref() { return *this; }

PLogLevel CLogger::lever() const { return level_; }

LogDump::LogDump() {}

void LogDump::operator+=(const CLogger &logger) {
  static std::mutex mutex;
  std::lock_guard<std::mutex> lock(mutex);
  std::cout << logger.toString() << "\033[0m" << std::endl;
}
