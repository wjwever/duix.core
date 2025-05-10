#pragma once

#include <onnxruntime_cxx_api.h>
#include <string>
#include <chrono>

namespace silero_vad{
class SileroVAD
{
private:
    // OnnxRuntime resources
    Ort::Env env;
    Ort::SessionOptions session_options;
    std::unique_ptr<Ort::Session> session = nullptr;
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU);

private:
    void init_engine_threads(int inter_threads, int intra_threads);

    void init_onnx_model(const std::string& model_path);
public:
    enum class SampleRate: uint32_t
    {
        SR_16K = 16000,
        SR_8K = 8000
    };
    enum class FrameMS : uint32_t{
        WS_32 = 32,
        WS_64 = 64,
        WS_96 = 96
    };

    /**
     * @brief Append one frame of audio to the buffer and detect speech
     * 
     * @param input_wav 
     * @return  start end none
     * @return 
     */
    std::string predict(const std::vector<float> &input_wav);

    /**
     * @brief Reset the states of the model
     * 
     */
    void Reset();


private:
    // model config
    uint32_t window_size_samples;  // Assign when init, support 256 512 768 for 8k; 512 1024 1536 for 16k.
    uint32_t sample_rate;  //Assign when init support 16000 or 8000      
    float threshold; 
    uint32_t min_silence_samples; // sr_per_ms * #ms
    uint32_t min_speech_samples; // sr_per_ms * #ms
    uint32_t speech_pad_samples; // usually a 
    uint32_t audio_length_samples;

    // model states
    bool triggered = false; 
    unsigned int temp_end = 0;
    unsigned int current_sample = 0;    
    // MAX 4294967295 samples / 8sample per ms / 1000 / 60 = 8947 minutes  
    int prev_end;
    int next_start = 0;

    // Onnx model
    // Inputs
    std::vector<Ort::Value> ort_inputs;
    
    std::vector<const char *> input_node_names = {"input", "sr", "h", "c"};
    std::vector<float> input;
    std::vector<int64_t> sr;
    unsigned int size_hc = 2 * 1 * 64; // It's FIXED.
    std::vector<float> _h;
    std::vector<float> _c;

    int64_t input_node_dims[2] = {}; 
    const int64_t sr_node_dims[1] = {1};
    const int64_t hc_node_dims[3] = {2, 1, 64};

    // Outputs
    std::vector<Ort::Value> ort_outputs;
    std::vector<const char *> output_node_names = {"output", "hn", "cn"};

public:
    // Construction
    SileroVAD(const std::string& ModelPath,
        SampleRate Sample_rate = SampleRate::SR_16K, FrameMS window_frame_ms = FrameMS::WS_32,
        float Threshold = 0.5, const std::chrono::milliseconds& min_silence_duration_ms = std::chrono::milliseconds(200),
        const std::chrono::milliseconds& speech_pad_ms = std::chrono::milliseconds(30), 
        const std::chrono::milliseconds& min_speech_duration_ms = std::chrono::milliseconds(64),
        const std::chrono::seconds&  max_speech_duration_s = std::chrono::seconds(std::numeric_limits<int64_t>::max()));
};
}
