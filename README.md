# duix.ai.core
Hello everyone, this project is dedicated to a **lightweight and cheap** digital human dialogue system, with the goal of running easily on a machine without a GPU.
Here is our example:

<table class="center">
<tr>
    <td heigh=50vh width=50% style="border: none">
        <video controls loop src="https://github.com/user-attachments/assets/fc45527c-1014-4d2e-afde-cd5123e3bb9f" muted="false"></video>
    </td>
</tr>
</table>

below is our dependencies, and now is tested under ubuntu22.04 and python3.10+
* avatar: [GitHub - GuijiAI/duix.ai](https://github.com/GuijiAI/duix.ai)
* vad: [silero-vad](https://github.com/snakers4/silero-vad)
* asr: [SenseVoice](https://github.com/FunAudioLLM/SenseVoice/)
* tts: [minimax](https://hailuoai.com/audio)
* lm: kimi, you can try deepseek and other language models

## ubuntu 22.04 
```bash
git clone --recurse-submodules https://github.com/wjwever/duix.ai.core.git 
cd duix.ai.core

#start avatar websocket server at port 6001, directory duix.ai.core/build
#remember to user your apikey in conf/config.json, current apiKey is only for test
bash start_avatar.sh

#start asr websocket server at port 6002
bash audio/start_audio_server.sh

#start ui , open http://localhost:6003 by google browser and give it a trye
bash web/start.sh

```
## macos
Not tested

# TODOS
- [x] use [kikoro tts](https://github.com/remsky/Kokoro-FastAPI)




