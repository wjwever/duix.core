/*************************************************************************

    > File Name: edge_render.cpp
    > Author: wangjiawei
    > Mail: wangjiawei07@baidu.com
    > Created Time: 2025年03月11日 星期二 22时50分37秒
 ************************************************************************/

#include <edge_render.h>
#include <filesystem>
#include <clog.h>
#include "aesmain.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <opencv2/opencv.hpp>
#include <wav_reader.h>


namespace fs = std::filesystem;
using json = nlohmann::json;

EdgeRender::EdgeRender() {
    _baseMD5Map = {
        {"alpha_model.b", "ab"},
        {"alpha_model.p", "ap"},
        {"cacert.p", "cp"},
        {"weight_168u.b", "wb"},
        {"wenet.o", "wo"}
    };
    _modelMD5Map = {
        {"dh_model.b", "db"},
        {"dh_model.p", "dp"},
        {"bbox.j", "bj"},
        {"config.j", "cj"},
        {"weight_168u.b", "wb"}

    };
}

int EdgeRender::load(const std::string& baseDir, const std::string& modelDir) {
    for (const auto& p : _baseMD5Map) {
        auto& key = p.first;
        auto& value = p.second;
        fs::path file = baseDir + "/" + value;
        if (fs::exists(file) == false) {
            fs::path newFile = baseDir + "/" +key;
            PLOGD << "convert " << newFile.string() << " to " << file.string();
            if (!fs::exists(newFile)) {
                PLOGD << "cant find " << newFile.string();
                return -1;
            }
            int ret = mainenc(0, const_cast<char*>(newFile.string().c_str()), const_cast<char*>(file.string().c_str()));
            PLOGD << "convert result:" << ret;
        }
    }
    for (const auto& p : _modelMD5Map) {
        auto& key = p.first;
        auto& value = p.second;
        fs::path file = modelDir + "/" + value;
        if (fs::exists(file) == false) {
            fs::path newFile = modelDir + "/" +key;
            PLOGD << "convert " << newFile.string() << " to " << file.string();
            if (!fs::exists(newFile)) {
                PLOGD << "cant find " << newFile.string();
                return -1;
            }
            int ret = mainenc(0, const_cast<char*>(newFile.string().c_str()), const_cast<char*>(file.string().c_str()));
            PLOGD << "convert result:" << ret;
        }
    }
    std::ifstream bbox(modelDir + "/" + _modelMD5Map["bbox.j"]);
    json boxJson = json::parse(bbox);

    std::ifstream config(modelDir + "/" + _modelMD5Map["config.j"]);
    json configJson = json::parse(config);

    _modelInfo._hasMask = configJson.value("need_png", 0) == 0
        and fs::exists(modelDir + "/raw_jpgs")
        and fs::exists(modelDir + "/raw_sg")
        and fs::exists(modelDir + "/pha");

    PLOGD << "hasMask:" << _modelInfo._hasMask;
    for (int i = 1; ; ++i) {
        auto rawPath = modelDir + "/raw_jpgs/" + std::to_string(i) + ".sij";
        auto maskPath = modelDir + "/pha_jpgs/" + std::to_string(i) + ".sij";
        auto sgPath = modelDir + "/raw_sg/" + std::to_string(i) + ".sij";
        if (fs::exists(rawPath) == false) {
            break;
        }
        Frame frame;
        frame.index = i;
        frame._rawPath = rawPath;
        if (_modelInfo._hasMask and fs::exists(maskPath)) {
            frame._maskPath = maskPath;
        }
        if (_modelInfo._hasMask and fs::exists(sgPath)) {
            frame._sgPath = sgPath;
        }
        if (boxJson.count(std::to_string(i))) {
            frame.rect[0] = boxJson[std::to_string(i)][0];
            frame.rect[1] = boxJson[std::to_string(i)][2];
            frame.rect[2] = boxJson[std::to_string(i)][1];
            frame.rect[3] = boxJson[std::to_string(i)][3];
        }
        //PLOGD << frame.toString();
        _modelInfo._frames.push_back(frame);
    }
    if (_modelInfo._frames.size() == 0) {
        return -2;
    }

    _modelInfo._width = configJson.value("width", 0);
    _modelInfo._height = configJson.value("height", 0);

    //  TODO 处理动作

    json ncnnConfig;
    ncnnConfig["action"] = 1;
    ncnnConfig["videowidth"] = _modelInfo._width;
    ncnnConfig["videoheight"] = _modelInfo._height;
    ncnnConfig["timeoutms"] = 5000;
    ncnnConfig["wenetfn"] = baseDir + "/wo";
    if (fs::exists(modelDir + "/wb")) {
        PLOGD << "使用模型自带的weight:" << modelDir + "/wb";      
        ncnnConfig["unetmsk"] = modelDir + "/wb";
    } else {
        ncnnConfig["unetmsk"] = baseDir + "/wb";
    }
    ncnnConfig["cacertfn"] = baseDir + "/cp";
    ncnnConfig["alphabin"] = baseDir + "/ab";
    ncnnConfig["alphaparam"] = baseDir + "/ap";

    ncnnConfig["unetbin"] = modelDir + "/db";
    ncnnConfig["unetparam"] = modelDir + "/dp";
    PLOGD << "ncnnConfig:" << ncnnConfig.dump(); 

    _modelInfo._ncnnConfig = ncnnConfig.dump();

    //TODO cost
    MessageCb* cb = nullptr;
    _digit = std::make_unique<GDigit>(_modelInfo._width, _modelInfo._height, cb);

    PLOGD <<"digit config:" << _digit->config(_modelInfo._ncnnConfig.c_str());
    _digit->start();
    PLOGD << "width:" << _modelInfo._width << " height:" << _modelInfo._height;
    PLOGD << "模型初始化完成";
    
    return 0;
}


int EdgeRender::fixWavHeader(const std::string& wav) {
    //TODO
    return 0;
}

int EdgeRender::render(const std::string& wav) {
    //fixWavHeader(wav);

    // frames
    std::string tmp = "./abcdefg.wav";
    int all_buf = _digit->newwav(tmp.c_str(), "");

    sleep(5);
    double wavDuration = durationMs(wav);
    PLOGD << "all_buf:" << all_buf << " wavDuration:" << wavDuration;



    cv::Mat mat = cv::Mat(_modelInfo._height, _modelInfo._width, CV_8UC3);
    for (int i = 0; i * 40 <wavDuration; ++i) {
        Frame frame = _modelInfo._frames[i % _modelInfo._frames.size()];
        //PLOGD << "index:" << i << " frame:" << frame.toString();
        
        _digit->onerstbuf(i, frame._rawPath.c_str(),  frame.rect, reinterpret_cast<char*>(mat.data), _modelInfo._height * _modelInfo._width * 3);
        //_digit->drawonebuf(frame._rawPath.c_str(),  reinterpret_cast<char*>(mat.data), _modelInfo._height * _modelInfo._width * 3);


        cv::Point topLeft(frame.rect[0], frame.rect[1]);
        cv::Point bottomRight(frame.rect[2], frame.rect[3]);

        cv::Scalar color(255, 0, 0);
        int thickness = 3;

        char buffer[50] = {0};
        snprintf(buffer, sizeof(buffer), "out/%03d.png", i);

        if (cv::imwrite(buffer, mat) == false) {
            //PLOGD << "Error saving image.";
        } else {
            //PLOGD << "saving image " << buffer;
        }
    }

    return 0;
}
