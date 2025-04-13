#pragma once
#include <iostream>
#include <string>
#include <curl/curl.h>

// 回调函数，用于接收HTTP响应数据
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

inline std::string getPublicIP() {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        // 使用ipify.org服务获取IP
        curl_easy_setopt(curl, CURLOPT_URL, "https://ipinfo.io/ip");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK) {
            return "Error: " + std::to_string(res);
        }

        return readBuffer;
    }
    return "Failed to initialize CURL";
}

/*
int main() {
    std::string publicIP = getPublicIP();
    std::cout << "Public IP: " << publicIP << std::endl;
    return 0;
}
*/
