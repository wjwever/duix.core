/*************************************************************************
    > File Name: infer.cpp
    > Author: frank
    > Mail: 1216451203
    > Created Time: 2025年05月07日 星期三 17时32分49秒
 ************************************************************************/
#include "sense_voice.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#include "kaldi-native-fbank/csrc/online-feature.h"
#include "kaldi-native-fbank/csrc/feature-fbank.h"
#include "kaldi-native-fbank/csrc/feature-window.h"
#include "kaldi-native-fbank/csrc/mel-computations.h"

// for time cost
#include "util.h"

std::vector<std::string> splitString(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}


SenseVoice::SenseVoice(const std::string& model_path, const std::string& tokens_path) {   
    env_ = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "test");
    session_options_.SetIntraOpNumThreads(1);                                                                
    session_options_.SetInterOpNumThreads(1);   
    session_ = std::make_unique<Ort::Session>(env_, model_path.c_str(), session_options_);                                 

    std::map<std::string, std::string> meta;
    getCustomMetadataMap(meta);

    auto get_int32 = [&meta](const std::string& key) {
        return stoi(meta[key]);
    };

    window_size_ = get_int32("lfr_window_size");
    window_shift_= get_int32("lfr_window_shift");

    std::vector<std::string> keys {
        "lang_zh", 
        "lang_en", 
        "lang_ja", 
        "lang_ko", 
        "lang_auto"
    };

    for (auto& key :keys) {
        lang_id_[key] = get_int32(key);
    }

    with_itn_ = get_int32("with_itn");
    without_itn_ = get_int32("without_itn");

    auto tmp = splitString(meta["neg_mean"], ',');
    for (auto f : tmp) {
        neg_mean_.push_back(stof(f));
    }
    tmp = splitString(meta["inv_stddev"], ',');
    for (auto f :tmp) {
        inv_stddev_.push_back(stof(f));
    }

    std::ifstream fin(tokens_path);
    std::string line;
    while(std::getline(fin, line)) {
        auto arr = splitString(line, ' ');
        if (arr.size() == 2) {
            tokens_[arr[1]] =  arr[0];
        }
    }

    setupIO();
}

SenseVoice::~SenseVoice() {
    /*
    for (auto name : input_names_) {
        delete[] name;
    }
    for (auto name : output_names_) {
        delete[] name;
    }
    */
}

void SenseVoice::setupIO() {
    Ort::AllocatorWithDefaultOptions allocator;

    // 获取输入信息
    size_t num_input_nodes = session_->GetInputCount();
    input_names_.reserve(num_input_nodes);

    for (size_t i = 0; i < num_input_nodes; i++) {
        auto input_name = session_->GetInputNameAllocated(i, allocator);

        char* dest = new char[strlen(input_name.get()) + 1]; // +1 用于空终止符
        input_names_.push_back(dest);
        strcpy(dest, input_name.get());

        Ort::TypeInfo type_info = session_->GetInputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

        std::vector<int64_t> input_dims = tensor_info.GetShape();
        std::cout << "Input " << i << " name: " << dest << std::endl;
        std::cout << "Input shape: ";
        for (auto dim : input_dims) {
            std::cout << dim << " ";
        }
        input_dims_.push_back(input_dims);
        std::cout << std::endl;
    }

    // 获取输出信息
    size_t num_output_nodes = session_->GetOutputCount();
    output_names_.reserve(num_output_nodes);

    for (size_t i = 0; i < num_output_nodes; i++) {
        auto output_name = session_->GetOutputNameAllocated(i, allocator);
        char* dest = new char[strlen(output_name.get()) + 1];
        strcpy(dest, output_name.get());
        output_names_.push_back(dest);

        Ort::TypeInfo type_info = session_->GetOutputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

        std::vector<int64_t> output_dims = tensor_info.GetShape();
        std::cout << "Output " << i << " name: " << dest << std::endl;
        std::cout << "Output shape: ";
        for (auto dim : output_dims) {
            std::cout << dim << " ";
        }
        std::cout << std::endl;
    }
}

std::string SenseVoice::recog(const std::vector<float>& data) {
        Timer cost("AsrCost");
        // extract fbank
        knf::FbankOptions opts;
        opts.frame_opts.dither = 0;
        opts.frame_opts.snip_edges = false;
        opts.frame_opts.window_type = "hamming";
        opts.frame_opts.samp_freq = 16000;
        opts.mel_opts.num_bins = 80;

        knf::OnlineFbank fbank(opts);
        fbank.AcceptWaveform(16000, data.data(), data.size());
        fbank.InputFinished();


        int32_t n = fbank.NumFramesReady();
        #if 0
        for (int32_t i = 0; i != n; ++i)
        {
          const float *frame = fbank.GetFrame(i);
          for (int32_t k = 0; k != opts.mel_opts.num_bins; ++k)
          {
            os << frame[k] << ", ";
          }
          os << "\n";
        }

        std::cout << os.str() << "\n";
        #endif
        std::vector<float> feats;

        for (int i = 0; i + window_size_ <= n; i+=window_shift_) {
          for (int k = i * 80; k < (i + window_size_) * 80; k++)  {
              double value = fbank.GetFrame(k / 80)[k % 80];
              feats.push_back((value + neg_mean_[k % 560]) * inv_stddev_[k%560]);
          }
        }

#if 0
        std::ostringstream os;
        os << std::fixed << std::setprecision(5);

        for (int32_t i = 0; i != feats.size(); ++i) {
          {
            os << feats[i]; 
          }
          if (( i + 1) % 560 == 0 ) {
            os << "\n";
          } else {
            os << " ";
          }
        }
#endif
        auto asr = infer(feats);
        return asr;
}

std::string SenseVoice::infer(const std::vector<float>& fbank) {
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
        OrtAllocatorType::OrtArenaAllocator,
        OrtMemType::OrtMemTypeDefault);

#if 0
    std::vector<int64_t> x_shape {1, -1, 560};
    auto x_ort = Ort::Value::CreateTensor<float>(memory_info,
                                                 fbank.data(),
                                                 fbank.size(),
                                                 input_dims_[0].data(),
                                                 input_dims_[0].size());  _ 
#endif                                                

    // x
    std::vector<int64_t> dims;
    dims.push_back(1);
    dims.push_back(fbank.size() / 560);
    dims.push_back(560);

    auto  x_ort = Ort::Value::CreateTensor<float>(
        memory_info,
        const_cast<float*>(fbank.data()),
        fbank.size(),
        dims.data(),
        dims.size());

    // x_lenght
    dims.clear();
    dims.push_back(1);
    int32_t x_length = fbank.size() /  560;
    auto x_length_ort = Ort::Value::CreateTensor<int32_t>(
        memory_info,
        &x_length,
        1,
        dims.data(),
        dims.size());

    dims.clear();
    dims.push_back(1);
    int32_t lang = lang_id_["lang_zh"];
    auto lang_ort = Ort::Value::CreateTensor<int32_t>(
        memory_info,
        &lang,
        1,
        dims.data(),
        dims.size());

    dims.clear();
    dims.push_back(1);
    int32_t text_norm = without_itn_;
    auto text_norm_ort = Ort::Value::CreateTensor<int32_t>(
        memory_info,
        &text_norm,
        1,
        dims.data(),
        dims.size());

    std::vector<Ort::Value> ort_inputs; 
    ort_inputs.emplace_back(std::move(x_ort));
    ort_inputs.emplace_back(std::move(x_length_ort));
    ort_inputs.emplace_back(std::move(lang_ort));
    ort_inputs.emplace_back(std::move(text_norm_ort));


    // 运行推理
    std::vector<Ort::Value> output_tensors = session_->Run(
        Ort::RunOptions{nullptr},
        input_names_.data(),
        ort_inputs.data(),
        ort_inputs.size(),
        output_names_.data(),
        output_names_.size());

    // 处理输出
    if (output_tensors.empty() || !output_tensors.front().IsTensor()) {
        throw std::runtime_error("Invalid output tensors");
    }



    auto info = output_tensors.front().GetTensorTypeAndShapeInfo();
    std::vector<int64_t> shape = info.GetShape();
    size_t dim_count = info.GetDimensionsCount();  // 获取维度数量
    std::cout << "dim_count:" << dim_count <<std::endl;
    std::cout << "shape: ";
    for (auto s :shape) {
        std::cout << s << " ";
    }
    std::cout <<std::endl;

    ONNXTensorElementDataType type = info.GetElementType();
    std::cout << "type:" << type << std::endl;

    // 3. 获取数据指针和元素数量
    float *logits_data = output_tensors.front().GetTensorMutableData<float>();
    size_t element_count = info.GetElementCount();

    // 4. 计算最后一个维度的大小
    int64_t last_dim = shape.empty() ? 1 : shape.back();
    size_t num_rows = element_count / last_dim;

    // 5. 为结果分配空间
    std::vector<int64_t> result(num_rows);

    // 6. 对每行计算 argmax
    for (size_t i = 0; i < num_rows; ++i)
    {
        float *row_start = logits_data + i * last_dim;
        result[i] = std::distance(
            row_start,
            std::max_element(row_start, row_start + last_dim));
    }

    std::vector<int64_t> final = unique_consecutive<int64_t>(result);
    std::string asr;
    for (const auto f :final) {
        if (f > 0 and f < 24884) {
            asr += tokens_[std::to_string(f)];
        }
    }
    return asr;
}

void SenseVoice::getCustomMetadataMap(std::map<std::string, std::string>& data) {
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::ModelMetadata model_metadata = session_->GetModelMetadata();
    
    // 获取自定义元数据数量
    auto keys = model_metadata.GetCustomMetadataMapKeysAllocated(allocator);
    std::cout << "\nCustom Metadata (" << keys.size() << " items):" << std::endl;
    
    // 遍历所有自定义元数据
    for (size_t i = 0; i < keys.size(); ++i) {
        const char* key = keys[i].get();
        auto value = model_metadata.LookupCustomMetadataMapAllocated(key, allocator);
        std::cout << key << ":" << value.get() << std::endl;
        data[std::string(key)] = std::string(value.get());
    }
}
