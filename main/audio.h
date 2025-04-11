#pragma once
#include <cstdlib>
#include <string>

using std::string;

inline bool mp3ToWavSystem(const string &mp3Path, const string &wavPath) {
  string command = "ffmpeg -i " + mp3Path + " " + wavPath;

  int result = system(command.c_str());
  return result == 0;
}
