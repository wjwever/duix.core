import asyncio
import websockets
import wave
import json
import time
from collections import deque
from datetime import datetime
import os
from silvad import FunASR, SileroVAD 
import uuid

import librosa
import numpy as np

def resample_bytes_to_16k(audio_np,  original_sr: int = 44100):
    """
    将 bytes 格式的 44.1kHz PCM 音频降采样到 16kHz
    
    参数:
        audio_bytes: 原始PCM字节数据（16位有符号整数）
        original_sr: 原始采样率（默认44100）
    
    返回:
        降采样后的PCM字节数据（16位有符号整数）
    """
    # 将bytes转换为numpy数组（假设是16位有符号整数）
    # audio_np = np.frombuffer(audio_bytes, dtype=np.int16)
    
    # 转换为float32（librosa需要）
    audio_float = audio_np.astype(np.float32) / 32768.0  # 归一化到[-1, 1]
    
    # 使用librosa重采样
    resampled = librosa.resample(
        audio_float, 
        orig_sr=original_sr, 
        target_sr=16000
    )
    
    # 转回int16并返回bytes
    resampled_int16 = (resampled * 32768).astype(np.int16)
    return resampled_int16

VAD = SileroVAD()
ASR = FunASR()

# Configuration
SAVE_INTERVAL = 2.0  # Save every 1 second
last_save_time = time.time()

# Ensure output directory exists
os.makedirs("audio_output", exist_ok=True)

AUDIO_BUFFER = np.zeros(1024, dtype=np.int16)
CUR_BUFFER =  np.array([0], dtype=np.int16)
state = "none"
totalLen = 0

PAD = 5

async def handle_audio(websocket):
    global last_save_time, AUDIO_BUFFER,totalLen, state
    current_sample_rate = 44100  # Default, will be updated from client
    
    wf = wave.open("last.wav", 'wb') 
    wf.setnchannels(1)  # Mono
    wf.setsampwidth(2)   # 2 bytes (16 bits) per sample
    wf.setframerate(16000)

    AUDIO_BUFFER = np.zeros(PAD * 512, dtype=np.int16)
    CUR_BUFFER =  np.array([0], dtype=np.int16)
    state = "none"
    totalLen = 0
    
    print("Client connected")
    try:
        async for message in websocket:
            try:
                # Parse the JSON message with metadata and audio data
                data = json.loads(message)
                metadata = data['metadata']
                
                # Update sample rate from metadata
                current_sample_rate = metadata['sampleRate']
                #print(f"Received audio chunk - SR: {current_sample_rate}Hz, TS: {metadata['timestamp']}")

                if 'audioData' not in data:
                    print('mute')
                    continue
                
                audio_data = data['audioData']
                audio_array = np.array(audio_data, dtype=np.int16)
                # audio_bytes = bytes(audio_data)
                int16_np = resample_bytes_to_16k(audio_array)
                totalLen += len(int16_np.tobytes()) // 2
                wf.writeframes(int16_np.tobytes())

                AUDIO_BUFFER = np.concatenate((AUDIO_BUFFER, int16_np))
                msgs = save_audio_to_wav(16000)
                if len(msgs) > 0:
                    res = json.dumps({'event':'query', 'value':msgs}, ensure_ascii=False)
                    await websocket.send(res)
                
                # current_time = time.time()
                # if current_time - last_save_time >= SAVE_INTERVAL:
                #     save_audio_to_wav(current_sample_rate)
                #     last_save_time = current_time
                    
            except json.JSONDecodeError:
                print("Received non-JSON message")
            except KeyError as e:
                print(f"Missing key in message: {e}")
                
    except websockets.exceptions.ConnectionClosed as e:
        print(f"连接关闭 代码: {e.code} 原因: {e.reason}")
        wf.close()

    finally:
        # Save any remaining audio when client disconnects
        wf.close()
        msgs = save_audio_to_wav(16000)
        if len(msgs) > 0:
            res = json.dumps({'event':'query', 'value':msgs}, ensure_ascii=False)
            await websocket.send(res)

def save_audio_to_wav(sample_rate) -> str:
    msgs = ""
    global AUDIO_BUFFER,CUR_BUFFER,totalLen,state
        
    # Convert deque to numpy array
    #audio_array = np.array(AUDIO_BUFFER, dtype=np.int16)
    #(length, ) = audio_array.shape
    (length, ) = AUDIO_BUFFER.shape
    
    for i in range(PAD, length // 512):
        res = VAD.is_vad(AUDIO_BUFFER[i * 512: i * 512+ 512])
        if res:
            if 'start' in res:
                state = 'start'
                CUR_BUFFER = AUDIO_BUFFER[(i - PAD)*512:i*512+512]
            elif 'end' in res:
                state = 'end'
                CUR_BUFFER = np.concatenate((CUR_BUFFER, AUDIO_BUFFER[i * 512: i * 512+ 512]), axis=0)
                
                timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
                filename = f"audio_output/audio_{timestamp}_{sample_rate}Hz.wav"
                # Save as WAV file
                with wave.open(filename, 'wb') as wf:
                    wf.setnchannels(1)  # Mono
                    wf.setsampwidth(2)   # 2 bytes (16 bits) per sample
                    wf.setframerate(16000)
                    wf.writeframes(CUR_BUFFER.tobytes())
                text = ASR.recognizer(filename)
                if len(text) > 0:
                    res = json.dumps({'event':'query', 'value':text}, ensure_ascii=False)
                    print(filename, res)
                    msgs += text
        elif state == 'start':
            CUR_BUFFER = np.concatenate((CUR_BUFFER, AUDIO_BUFFER[i * 512: i * 512+ 512]), axis=0)
        print(f'total:{totalLen} cur: {CUR_BUFFER.shape}')
    AUDIO_BUFFER = AUDIO_BUFFER[(length // 512 - PAD) * 512:]
    return msgs
async def main():
    server = await websockets.serve(
        handle_audio,
        "0.0.0.0",
        8765,
        ping_interval=None,
        ping_timeout=None    # 无超时限制
    )
    print("WebSocket server started on ws://0.0.0.0:8765")
    await server.wait_closed()

if __name__ == "__main__":
    asr = ASR.recognizer("test.wav")
    print(asr)
    if asr != "关注自我成长，享受生活。":
        print("asr self test error, please check")
        exit(0)
    asyncio.get_event_loop().run_until_complete(main())
