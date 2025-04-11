/*************************************************************************
    > File Name: wav_reader.h
    > Author: wangjiawei
    > Mail: 1216451203@qq.com
    > Created Time: 2025年03月11日 星期二 22时52分59秒
 ************************************************************************/
#pragma once

#include "clog.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>

struct WAVHeader {
  char chunkId[4];    // RIFF
  uint32_t chunkSize; // 文件大小 4字节
  char format[4];     // "WAVE"

  // fmt 子块
  char subchunk1ID[4];    // "fmt "
  uint32_t subchunk1Size; // 16 for pcm
  uint16_t audioFormat;   // 音频格式 1=pcm
  uint16_t numChannels;   // 通道数
  uint32_t sampleRate;    // 采样率
  uint32_t byteRate; // 每秒字节数 sampleRate * numChannels * bitsPerSample / 8
  uint16_t blockAlign;    // 每采样帧字节数 numChannels * bitsPerSample / 8
  uint16_t bitsPerSample; // 采样率

  char subchunk2ID[4];    // "data"
  uint32_t subchunk2Size; // 数据大小 = 采样数 * 通道数 * 位深度 / 8
  void to_string() {
    PLOGI << "chunkId:" << std::string(chunkId, 4) << " chunkSize:" << chunkSize
          << " format:" << std::string(format, 4)
          << " subchunk1ID:" << std::string(subchunk1ID, 4)
          << " subchunk1Size:" << subchunk1Size
          << " audioFormat:" << audioFormat << " numChannels:" << numChannels
          << " sampleRate:" << sampleRate << " bits:" << bitsPerSample
          << " subchunk1ID:" << std::string(subchunk2ID, 4)
          << " subchunk2Size:" << subchunk2Size;
  }
};

inline double durationMs(const std::string &wav) {
  std::ifstream file(wav.c_str(), std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to open WAV file." << std::endl;
    return -1.0;
  }
  WAVHeader header;
  file.read(reinterpret_cast<char *>(&header), sizeof(WAVHeader));
  header.to_string();

  // 校验WAV文件头
  if (std::string(header.chunkId, 4) != "RIFF" ||
      std::string(header.format, 4) != "WAVE") {
    return -2.0;
  }

  // 计算时长(ms)
  uint32_t dataSize = header.chunkSize - 8;      //
  uint32_t sampleRate = header.sampleRate;       //
  uint16_t numChannels = header.numChannels;     //
  uint16_t bitsPerSample = header.bitsPerSample; //

  double durationMs = (static_cast<double>(dataSize) /
                       (sampleRate * numChannels * (bitsPerSample / 8))) *
                      1000;

  file.close();
  return durationMs;
}

inline std::string fixWav(const std::string &wav) {
  std::ifstream file(wav.c_str(), std::ios::binary);
  if (!file.is_open()) {
    PLOGE << "Failed to open WAV file." << wav;
    return "";
  }
  // 文件指针移动到末尾位置
  file.seekg(0, std::ios_base::end);

  // 获取当前文件指针位置，也就是获得文件大小
  std::streampos fileSize = file.tellg();

  // 文件指针移动到开头文件
  file.seekg(0, file.beg);

  WAVHeader header;
  file.read(reinterpret_cast<char *>(&header), sizeof(WAVHeader));
  if (fileSize != header.chunkSize + 8) {
    std::string newWav = std::filesystem::path(wav).parent_path().string() +
                         "/fix_" +
                         std::filesystem::path(wav).filename().string();

    PLOGD << "fix " << wav << "to" << newWav;

    char *content = new char[fileSize];
    file.seekg(0, file.beg);
    file.read(content, fileSize);
    file.close();

    int setChunk2Size = int(fileSize) - 8;
    content[7] = setChunk2Size >> 24;
    content[6] = (setChunk2Size << 8) >> 24;
    content[5] = (setChunk2Size << 16) >> 24;
    content[4] = (setChunk2Size << 24) >> 24;

    if (std::string(header.subchunk2ID, 4) == "data") {
      int setChunk2Size = int(fileSize) - 44;
      content[43] = (setChunk2Size >> 24);
      content[42] = ((setChunk2Size << 8) >> 24);
      content[41] = ((setChunk2Size << 16) >> 24);
      content[40] = ((setChunk2Size << 24) >> 24);
    }

    std::ofstream outfile(newWav.c_str(), std::ofstream::binary);
    outfile.write(content, fileSize);
    outfile.close();
    return newWav;
  }
  PLOGD << "fix " << wav << " to" << wav;
  return wav;
}
