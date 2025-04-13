# 简介
* 一直想找一个低成本（cpu可驱动）的数字人驱动方案，调研到硅基智能的[GitHub - GuijiAI/duix.ai](https://github.com/GuijiAI/duix.ai)，体验下来数字人播报效果还不错、关键是人家只需要cpu就能跑！！
* 于是基于官方的android sdk，改造了可以在pc上运行的代码，方便后续扩展其他使用场景。
* 遗憾的是，目前仅支持支持使用官方提供的形象，定制新形象需要给硅基额外付费。


<table class="center">
    
<tr>
    <td width=50% style="border: none">
        <video controls loop src="https://github.com/user-attachments/assets/fc45527c-1014-4d2e-afde-cd5123e3bb9f" muted="false"></video>
    </td>
    <td width=50% style="border: none">
        <video controls loop src="https://github.com/user-attachments/assets/556f4e7f-5191-49eb-bc39-6bf8c991d7fd" muted="false"></video>
    </td>
</tr>

</table>


## ubuntu 22.04

### 安装

```bash
sudo apt update
sudo apt install build-essential cmake  wget curl  ffmpeg git libopencv-dev libcurl4-openssl-dev
git clone --recurse-submodules https://github.com/wjwever/duix.ai.core.git 
cd duix.ai.core
mkdir -p build
cd build
cmake ../
make -j 3
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

# 扩展

- [ ] 定制新形象需要收费，需要找到一个支持扩展形象的方案
- [ ] 卡通形象支持
- [ ] 支持macos
- [x] 数字人短视频合成，https://github.com/wjwever/avatar_short_video
- [ ] 数大模型实时驱动

