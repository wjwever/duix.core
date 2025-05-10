#ifndef INFERENCE_ENGINE_H
#define INFERENCE_ENGINE_H

#include <string>
#include <vector>
#include <map>
#include <onnxruntime_cxx_api.h>
#include <memory>


std::vector<std::string> splitString(const std::string& s, char delimiter);

template <typename T>
std::vector<T> unique_consecutive(const std::vector<T>& input) {
    if (input.empty()) return {};
    
    std::vector<T> result;
    result.push_back(input[0]);
    
    for (size_t i = 1; i < input.size(); ++i) {
        if (input[i] != input[i-1]) {
            result.push_back(input[i]);
        }
    }
    
    return result;
}

class SenseVoice {
public:
    SenseVoice(const std::string& model_path, const std::string& token_path);
    ~SenseVoice();

    std::string recog(const std::vector<float>& wav);
    
    std::string infer(const std::vector<float>& feat);
    int32_t window_size_;
    int32_t window_shift_;
    int32_t with_itn_;
    int32_t without_itn_;

    std::map<std::string, int32_t> lang_id_;
    std::vector<float> neg_mean_;
    std::vector<float> inv_stddev_;
    std::map<std::string, std::string> tokens_;
    
private:
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    std::unique_ptr<Ort::Session> session_;
    
    std::vector<const char*> input_names_;
    std::vector<std::vector<int64_t>> input_dims_;
    std::vector<const char*> output_names_;
    
    void setupIO();
    void getCustomMetadataMap(std::map<std::string, std::string>& data);


#if 0
    int32_t lang_zh_;
    int32_t lang_en_;
    int32_t lang_ja_;
    int32_t lang_ko_;
    int32_t lang_auto_;
#endif
};


#endif // INFERENCE_ENGINE_H
