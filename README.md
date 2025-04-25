<div class="column" align="middle">
  <p align="center">
  </p>
  </a>
  <a href="https://en.cppreference.com/w/">
    <img src="https://img.shields.io/badge/Language-C++-blue.svg" alt="language"/>
  </a>
  <img src="https://img.shields.io/badge/platform-Linux-9cf.svg" alt="linux"/>
  <img src="https://img.shields.io/badge/Release-v0.1.0-green.svg" alt="release"/>

<h4 align="center">If you are interested in This project, please kindly give Me a triple `Star`, `Fork` and `Watch`, Thanks!</h4>
</div>

# duix.ai.core
Hello everyone! </br>
This project is dedicated to a **lightweight and cheap** digital human dialogue system. </br>
**Lightweight**:  The project runs without gpus </br>
**Cheap**:  The project's functions should be deployed locally as much as possible instead of calling commercial APIs </br>

https://github.com/user-attachments/assets/40ad194b-370a-4a00-9264-67c796e43e12



Below are the key components of the project. The entire system was tested on Ubuntu 22.04.
* avatar: [GitHub - GuijiAI/duix.ai](https://github.com/GuijiAI/duix.ai)
* vad: [silero-vad](https://github.com/snakers4/silero-vad)
* asr: [SenseVoice](https://github.com/FunAudioLLM/SenseVoice/)
* tts: [minimax](https://hailuoai.com/audio)
* lm: kimi now， it is easy to try deepseek and some other language models，just to change the api url vi conf/config.json

## ubuntu 22.04 
```bash
git clone --recurse-submodules https://github.com/wjwever/duix.ai.core.git 
cd duix.ai.core

#start avatar websocket server at port 6001, directory duix.ai.core/build
#remember to user your apikey in conf/config.json, current apiKey is only for test
bash start_avatar.sh

#start asr websocket server at port 6002
bash audio/start_audio_server.sh

#start ui , open http://localhost:6003 by google browser and give it a try
bash web/start.sh

```
## macos
Not tested

# TODOS
- [x] use [kikoro tts](https://github.com/remsky/Kokoro-FastAPI) rather than commercial apis




