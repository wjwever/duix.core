/*************************************************************************
    > File Name: wav_reader.h
    > Author: wangjiawei
    > Mail: wangjiawei07@baidu.com
    > Created Time: 2025年03月11日 星期二 22时52分59秒
 ************************************************************************/
#include <iostream>
#include <fstream>
#include <cstdint>


struct WAVHeader {
    char chunkId[4];            // RIFF
    uint32_t chunkSize;         // 文件大小 8字节                        
    char format[4];             // "WAVE"                   

    // fmt 子块
    char subchunk1ID[4];        //"fmt "
    uint32_t  subchunk1Size;    // 16 for pcm
    uint16_t  audioFormat;      // 音频格式 1=pcm
    uint16_t  numChannels;      // 通道数
    uint32_t  sampleRate;       // 采样率                                      
    uint32_t  byteRate;         // 每秒字节数 sampleRate * numChannels * bitsPerSample / 8
    uint16_t  blockAlign;       // 每采样帧字节数 numChannels * bitsPerSample / 8
    uint16_t  bitsPerSample;    // 采样率                                      
                                
                               
    char subchunk2ID[4];        // "data"
    uint32_t subchunk2Size;     // 数据大小 = 采样数 * 通道数 * 位深度 / 8
};



double durationMs(const std::string& wav) {
    std::ifstream file(wav.c_str(), std::ios::binary);
    if (!file.is_open()) {
        std::cerr <<"Failed to open WAV file." << std::endl;
        return -1.0;
    }
    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    //校验WAV文件头
    if (std::string(header.chunkId, 4) != "RIFF" ||
        std::string(header.format, 4) != "WAVE" ||
        std::string(header.subchunk2ID, 4) != "data") {
            std::cerr << "Invalid wav file format." << std::endl;
            return -2.0;
    }

    // 计算时长(ms)
    uint32_t dataSize = header.subchunk2Size;       //
    uint32_t sampleRate = header.sampleRate;        //
    uint16_t numChannels = header.numChannels;      // 
    uint16_t bitsPerSample = header.bitsPerSample;   //  
                                                     
    std::cout << dataSize << " "
              << sampleRate << " "
              << numChannels << " "
              << bitsPerSample << std::endl;
    
    double durationMs = (static_cast<double>(dataSize) / 
                    (sampleRate * numChannels * (bitsPerSample / 8))) * 1000;

    file.close();
    return durationMs;
}
