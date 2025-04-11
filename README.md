# 简介

工作需要，想找一个低成本（cpu可驱动）的数字人驱动方案，调研到硅基智能的[GitHub - GuijiAI/duix.ai](https://github.com/GuijiAI/duix.ai)，体验下来口型效果、在cpu上驱动速度都还不错。于是基于官方的android sdk，改造了一版，可以在pc上运行的代码，方便后续扩展出其他的其他的用途。不过遗憾的是，目前仅支持支持使用官网提供的形象，定制新形象需要花钱。

<video width="320" height="240" controls>
  <source src="conf/hello.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

## ubuntu 22.04

### 安装

```bash
sudo apt install build-essential cmake  wget curl  ffmpeg git libopencv-dev libcurl4-openssl-dev
git clone --recurse-submodules https://github.com/wjwever/duix.core.git 
cd duix.core
mkdir -p build
cd build
cmake ../
make -j
cp ../conf . -r
```

编译完成以后，build目录会出现两个编译产物:offline server

### 运行

offline 输入音频驱动数字人，生成数字人短视频

```
./offline  --role=XiaoXuan --wav=conf/hello.wav
```

server 工作流程为文本->tts->数字人驱动，依赖[MiniMax-与用户共创智能](https://platform.minimaxi.com/document/T2A%20V2?key=66719005a427f0c8a5701643)] tts合成，需要先修改conf/conf.json，修改为正确的apiKe和groupId，修改后启动方式：

```bash
./server  --conf=conf/conf.json
```

服务会在8080端口监听http请求，http请求发送如下，预期会收到服务端流式消息，最后一行为video的地址

```bash
bash conf/curl.sh
```

## macos

* TODO

# 计划

- [ ] 添加前端
- [ ] 定制新形象需要收费，需要找到一个支持形象的方案
- [ ] 卡通形象支持
