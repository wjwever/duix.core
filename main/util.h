#pragma once

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <iostream>
#include <string.h>
#include <string>

inline std::string localIP() {
  std::string ip = "127.0.0.1";
  struct ifaddrs *ifaddr, *ifa;
  char ipstr[INET_ADDRSTRLEN];

  // 获取网络接口列表‌:ml-citation{ref="1,4" data="citationList"}
  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs failed");
    return ip;
  }

  // 遍历所有接口
  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;

    // 仅处理IPv4地址‌:ml-citation{ref="3,4" data="citationList"}
    if (ifa->ifa_addr->sa_family == AF_INET) {
      struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
      inet_ntop(AF_INET, &addr->sin_addr, ipstr, INET_ADDRSTRLEN);

      std::cout << "IPv4 Address (" << ifa->ifa_name << "): " << ipstr
                << std::endl;
      if (strcmp(ifa->ifa_name, "lo") != 0) {
        ip = ipstr;
        break;
      }
    }
  }

  freeifaddrs(ifaddr); // 释放资源
  return ip;
}
