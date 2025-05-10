#pragma once
#include <string>
#include <memory>
#include <deque>
#include <atomic>
#include <mutex>
#include <thread>
#include "sense_voice.h"
#include "vad.h"

using silero_vad::SileroVAD;

struct AsrMsg {
    std::string type = "";
    std::string msg = "";
};

class Asr {
    public:
        Asr(const std::string& asr_onnx, const std::string& tokens, const std::string& vad_onnx);
        ~Asr();
        void push_data(const std::vector<float>& data, int sampleRate);
        void run();
        std::atomic<bool> _running;
        std::thread _th;
        std::function<void(const std::string& asr)> _onAsr;

    private:
        std::unique_ptr<SileroVAD> _vad;
        std::unique_ptr<SenseVoice> _sence_voice;
        std::deque<float> _deque;
        std::vector<float> _curWav;
        std::mutex _mx;
};
