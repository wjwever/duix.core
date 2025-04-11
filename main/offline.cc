/*************************************************************************
    > File Name: main.cpp
    > Author: wangjiawei
    > Mail: 1216451203@qq.com
    > Created Time: 2025年03月12日 星期三 20时59分11秒
 ************************************************************************/

#include "clog.h"
#include <edge_render.h>
#include <getopt.hpp>
#include <memory>
#include <string>
using namespace std;

int main() {
  std::string wav = getarg("", "-w", "--wav");
  std::string role = getarg("siyao", "-r", "--role");

  auto render = std::make_shared<EdgeRender>();
  PLOGD << "role: " << role;
  render->load(role);
  std::thread th(&EdgeRender::render, render.get(), wav);
  std::string data;
  while (render and render->done() == false) {
    render->getMsg(data);
    if (data.size() > 0) {
      PLOGD << data;
    }
  }

  th.join();

  return 0;
}
