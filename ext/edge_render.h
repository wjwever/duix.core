/*************************************************************************
    > File Name: edge_render.h
    > Author: wangjiawei
    > Mail: wangjiawei07@baidu.com
    > Created Time: 2025年03月11日 星期二 22时46分16秒
 ************************************************************************/
#pragma once
#include <string>
#include <map>
#include <memory>
#include <model_info.h>
#include <digit/GDigit.h>

class EdgeRender {
public:
    EdgeRender();
    int load(const std::string& baseDir, const std::string& modelDir);

    int render(const std::string& wav);
    int fixWavHeader(const std::string& wav);

    std::map<std::string, std::string> _baseMD5Map;
    std::map<std::string, std::string> _modelMD5Map;

    ModelInfo _modelInfo;
    std::unique_ptr<GDigit> _digit;
};
