#include <vad.h>

#include <iostream>
#include <vector>
#include <sstream>
#include <cstring>
#include <limits>
#include <chrono>
#include <memory>
#include <string>
#include <stdexcept>
#include <iostream>
#include <string>
#include <onnxruntime_cxx_api.h>
#include <cstdio>
#include <cstdarg>
#include <cassert>
// #ifdef __APPLE__
// #include <coreml_provider_factory.h>
// #endif

using namespace Ort;
using namespace silero_vad;
void SileroVAD::init_engine_threads(int inter_threads, int intra_threads)
{
    // The method should be called in each thread/proc in multi-thread/proc work
    session_options.SetIntraOpNumThreads(intra_threads);
    session_options.SetInterOpNumThreads(inter_threads);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
};

void SileroVAD::init_onnx_model(const std::string &model_path)
{
    // Init threads = 1 for
    init_engine_threads(1, 1);
    /*
    #ifdef __APPLE__
    uint32_t coreml_flags = 0;
    coreml_flags |= COREML_FLAG_ONLY_ENABLE_DEVICE_WITH_ANE;

    Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_CoreML(session_options, coreml_flags));
    #endif*/
    // Load model
    session = std::make_unique<Ort::Session>(env, model_path.c_str(), session_options);


};

void SileroVAD::Reset()
{
    // Call reset before each audio start
    std::memset(_h.data(), 0.0f, _h.size() * sizeof(float));
    std::memset(_c.data(), 0.0f, _c.size() * sizeof(float));
    triggered = false;
    temp_end = 0;
    current_sample = 0;

    prev_end = next_start = 0;
};

std::string SileroVAD::predict(const std::vector<float> &data)
{
    // Infer
    // Create ort tensors
    input.assign(data.begin(), data.end());
    Ort::Value input_ort = Ort::Value::CreateTensor<float>(
        memory_info, input.data(), input.size(), input_node_dims, 2);
    Ort::Value sr_ort = Ort::Value::CreateTensor<int64_t>(
        memory_info, sr.data(), sr.size(), sr_node_dims, 1);
    Ort::Value h_ort = Ort::Value::CreateTensor<float>(
        memory_info, _h.data(), _h.size(), hc_node_dims, 3);
    Ort::Value c_ort = Ort::Value::CreateTensor<float>(
        memory_info, _c.data(), _c.size(), hc_node_dims, 3);

    // Clear and add inputs
    ort_inputs.clear();
    ort_inputs.emplace_back(std::move(input_ort));
    ort_inputs.emplace_back(std::move(sr_ort));
    ort_inputs.emplace_back(std::move(h_ort));
    ort_inputs.emplace_back(std::move(c_ort));

    // Infer
    ort_outputs = session->Run(
        Ort::RunOptions{nullptr},
        input_node_names.data(), ort_inputs.data(), ort_inputs.size(),
        output_node_names.data(), output_node_names.size());

    // Output probability & update h,c recursively
    float speech_prob = ort_outputs[0].GetTensorMutableData<float>()[0];
    float *hn = ort_outputs[1].GetTensorMutableData<float>();
    std::memcpy(_h.data(), hn, size_hc * sizeof(float));
    float *cn = ort_outputs[2].GetTensorMutableData<float>();
    std::memcpy(_c.data(), cn, size_hc * sizeof(float));

    // Push forward sample index
    current_sample += window_size_samples;

    if (speech_prob >= threshold) {
        temp_end = 0; 
    } 
    if (speech_prob >= threshold and triggered == false){
        triggered = true;
        //int speech_start = current_sample - speech_pad_samples - window_size_samples;
        return "start"; 
    }
    if (speech_prob < threshold - 0.15 and triggered) {
        if (temp_end == 0) {
            temp_end = current_sample;
        }
        if (current_sample - temp_end < min_silence_samples) {
            return "none";
        } else {
            temp_end = 0;
            triggered = false;
            return "end";
        } 
    }

    return "none";
};


SileroVAD::SileroVAD(const std::string& ModelPath,
        SampleRate Sample_rate, FrameMS window_frame_ms,
        float Threshold , const std::chrono::milliseconds& min_silence_duration_ms,
        const std::chrono::milliseconds& speech_pad_ms, 
        const std::chrono::milliseconds& min_speech_duration_ms,
        const std::chrono::seconds&  max_speech_duration_s)
{
    init_onnx_model(ModelPath);
    threshold = Threshold;
    sample_rate = static_cast<uint32_t>(Sample_rate);
    int sr_per_ms = sample_rate / 1000;

    window_size_samples = static_cast<uint32_t>(window_frame_ms) * sr_per_ms;

    min_speech_samples = sr_per_ms * min_speech_duration_ms.count();
    speech_pad_samples = sr_per_ms * speech_pad_ms.count();

    min_silence_samples = sr_per_ms * min_silence_duration_ms.count();

    input.resize(window_size_samples);
    input_node_dims[0] = 1;
    input_node_dims[1] = window_size_samples;

    _h.resize(size_hc);
    _c.resize(size_hc);
    sr.resize(1);
    sr[0] = sample_rate;
};
