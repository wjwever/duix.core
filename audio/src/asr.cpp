#include "asr.h"
#include "iostream"
#include <samplerate.h>

static std::vector<float> resample(
    const std::vector<float>& input,
    int inputRate)
    //int outputRate,
    //int converterType)
{
    int outputRate = 16000;
    if (inputRate == outputRate) {
        return input;
    }


    if (input.empty()) return {};
    if (inputRate <= 0 || outputRate <= 0) {
        throw std::invalid_argument("Sample rates must be positive");
    }

    const double ratio = double(outputRate) / inputRate;
    std::vector<float> output(static_cast<size_t>(input.size() * ratio + 0.5));

    SRC_DATA src_data;
    src_data.data_in = const_cast<float*>(input.data());
    src_data.input_frames = static_cast<long>(input.size());
    src_data.data_out = output.data();
    src_data.output_frames = static_cast<long>(output.size());
    src_data.src_ratio = ratio;
    src_data.end_of_input = 1;

    int converterType = 2;
    int error = src_simple(&src_data, converterType, 1);
    if (error) {
        throw std::runtime_error("Resampling failed: " + std::string(src_strerror(error)));
    }

    output.resize(static_cast<size_t>(src_data.output_frames_gen));
    return output;
}

Asr::Asr(const std::string& asr_onnx, 
         const std::string& tokens, 
         const std::string& vad_onnx) 
{
    _running = true;
    _sence_voice = std::make_unique<SenseVoice>(asr_onnx, tokens);
    _vad = std::make_unique<SileroVAD>(vad_onnx);
    _th = std::thread(&Asr::run, this);
}

Asr::~Asr() {
    _running = false;
    _th.join();
}

void Asr::push_data(const std::vector<float>& data, int inputRate) {
    std::lock_guard<std::mutex> lk(_mx);
    auto out = resample(data, inputRate);
    for (const auto& d : out) {
        _deque.push_back(d);
    }
}

void Asr::run() {
    int idx = 0;
    while(_running.load()) {
        std::vector<float> data;
        {
            std::lock_guard<std::mutex> lk(_mx);
            data.clear(); 
            if (_deque.size() > 512) {
                for (int i = 0; i < 512; ++i) {
                    data.push_back(_deque.front());
                    _deque.pop_front();
                }
            }
        }
        if (data.size() == 512) {
            auto trigger = _vad->predict(data);
            if (trigger != "none") {
                std::cout << 512 * idx++ << " " << trigger  << std::endl;
            }
            // for asr detect
            std::transform(data.begin(), data.end(), data.begin(),
                    [](float x) { return x * 32768.0f; });
            if (trigger == "start") {  // detect voice
                _curWav.insert(_curWav.end(), data.begin(), data.end()); 
            } else if (trigger == "end"){  // detect silence
                _curWav.insert(_curWav.end(), data.begin(), data.end()); 
                auto result = _sence_voice->recog(_curWav);
                if (_onAsr) {
                    _onAsr(result);
                }
                _curWav.clear();
            } else if (_curWav.size() > 0) {
                _curWav.insert(_curWav.end(), data.begin(), data.end()); 
            }
        }
    }
    std::cout << "Asr run exit" <<std::endl;
}
