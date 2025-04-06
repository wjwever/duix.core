/*************************************************************************
    > File Name: main.cpp
    > Author: wangjiawei
    > Mail: wangjiawei07@baidu.com
    > Created Time: 2025年03月12日 星期三 20时59分11秒
 ************************************************************************/

#include <string>
#include <edge_render.h>
using namespace std;

static EdgeRender gRender;

int main() {
    
    std::string baseDir = "gj_dh_res", modelDir = "siyao_20240418";
    gRender.load(baseDir, modelDir);


    gRender.render("./abcdefg.wav");
    return 0;
}
