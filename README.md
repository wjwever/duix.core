<h1 align="center">duix.ai.core </h1>
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

Hello everyone! This project is dedicated to a **LightWeight and Cheap** digital human dialogue system. </br>
**LightWeight** means The project runs without gpus </br>
**Cheap** meas The project's functions should be deployed locally as much as possible instead of calling commercial APIs </br>

https://github.com/user-attachments/assets/40ad194b-370a-4a00-9264-67c796e43e12



Below are the key components of the project. The entire system was tested on Ubuntu 22.04.
* avatar: [GitHub - GuijiAI/duix.ai](https://github.com/GuijiAI/duix.ai)
* vad: [silero-vad](https://github.com/snakers4/silero-vad)
* asr: [SenseVoice](https://github.com/FunAudioLLM/SenseVoice/)
* tts: [minimax](https://hailuoai.com/audio)
* lm: kimi now， it is easy to try deepseek and some other language models，just to change the api url in conf/config.json

## Tested environment: ubuntu 22.04
```bash
git clone --recurse-submodules https://github.com/wjwever/duix.ai.core.git 
cd duix.ai.core

# Rtart avatar websocket server at port 6001, directory duix.ai.core/build
# Remember to user your apikey in conf/config.json, current apiKey is only for test
bash start_avatar.sh

#start asr websocket server at port 6002
bash audio/start_audio_server.sh

#start ui , open http://localhost:6003 by google browser and give it a try
bash web/start_web.sh

```
## macos
Not tested

## Docker Deployment(Building)
```bash
# Build the image
docker build -t duix-ai .
# Run the container (example)
docker run -d \
  -p 6001-6003:6001-6003 \
  -p 38080:8080 \
  -e MINIMAX_API_KEY=your_minimax_api_key \
  -e LM_API_KEY=your_lm_api_key \
  duix-ai
```

### Docker Configuration Details:
- Multi-stage build to minimize image size
- Integrated HuggingFace model download with mirror support
- Supervisord process management for multiple services
- Preconfigured Python environment with all dependencies

## FAQ
1. The first build requires downloading ~2GB model files, ensure stable network connection
2. If encountering model download issues, set HF_TOKEN environment variable:
```bash
   export HF_TOKEN=your_huggingface_token
```
3. Audio service requires these system dependencies:
   - portaudio19-dev
   - ffmpeg
4. For Python dependency conflicts, try rebuilding Docker image without cache:
```bash
   docker build --no-cache -t duix-ai .
```

# TODOS
- [x] use [kikoro tts](https://github.com/remsky/Kokoro-FastAPI) rather than commercial apis
- [ ] Optimize docker workflow




