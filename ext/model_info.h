/*************************************************************************
    > File Name: model_info.h
    > Author: wangjiawei
    > Mail: wangjiawei07@baidu.com
    > Created Time: 2025年03月12日 星期三 19时26分26秒
 ************************************************************************/
#pragma once

#include <string>
#include <vector>
#include <sstream>

struct Frame {
    int index = 0;
    std::string _rawPath;
    std::string _maskPath;
    std::string _sgPath;

    int rect[4] = {0};
    bool check() {
        return _rawPath.size() > 0;
    }

    std::string toString() {
        std::stringstream ss;
        ss << "frame:"  << index
           << " jpg:"   << _rawPath
           << " mask:"  << _maskPath
           << " sg:"    << _sgPath
           << " rect:"  << rect[0] << " " << rect[1] << " " << rect[2] << " " << rect[3];
        return ss.str();
    }
};


struct ModelInfo {
    bool _hasMask = true;
    std::vector<Frame> _frames;
    int _width = 0;
    int _height = 0;
    std::string _ncnnConfig = "";
};
