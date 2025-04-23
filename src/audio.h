#pragma once
#include <cstdlib>
#include <string>
#include <cstdio>


using std::string;

inline bool mp3ToWavSystem(const string &mp3Path, const string &wavPath) {
  string command = "ffmpeg -i " + mp3Path + " " + wavPath  + "> /dev/null 2>&1";

  int result = system(command.c_str());
  return result == 0;

/*
FILE* pipe = popen(cmd, "r");
if (!pipe) return false;
pclose(pipe);
return true;
*/
}
