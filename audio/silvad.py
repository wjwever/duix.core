from silero_vad import load_silero_vad, VADIterator
import numpy as np
import torch
import sys
import wave
import math
import time  
from funasr import AutoModel
from funasr.utils.postprocess_utils import rich_transcription_postprocess


# class ASR():
#     @staticmethod
#     def _save_audio_to_file(audio_data, file_path):
#         """将音频数据保存为WAV文件"""
#         try:
#             with wave.open(file_path, 'wb') as wf:
#                 wf.setnchannels(1)
#                 wf.setsampwidth(2)
#                 wf.setframerate(16000)
#                 wf.writeframes(b''.join(audio_data))
#             print(f"ASR识别文件录音保存到：{file_path}")
#         except Exception as e:
#             print(f"保存音频文件时发生错误: {e}")
#             raise
#
#     @abstractmethod
#     def recognizer(self, stream_in_audio):
#         """处理输入音频流并返回识别的文本，子类必须实现"""
#         pass


class FunASR():
    def __init__(self):
        self.model_dir = "models/SenseVoiceSmall"
        self.output_dir = "tmp"

        self.model = AutoModel(
            model=self.model_dir,
            vad_kwargs={"max_single_segment_time": 30000},
            disable_update=True,
            hub="hf"
            # device="cuda:0",  # 如果有GPU，可以解开这行并指定设备
        )
    def recognizer(self, audio):
        try:
            # res = self.model.generate(
            #     input=audio,
            #     cache={},
            #     language="zn",  # 语言选项: "zn", "en", "yue", "ja", "ko", "nospeech"
            #     use_itn=True,
            #     batch_size_s=60,
            # )

            res = self.model.generate(
                input = audio,
                cache={},
                language="zn",  # "zn", "en", "yue", "ja", "ko", "nospeech"
                use_itn=True,
                batch_size_s=60,
                merge_vad=True,  #
                merge_length_s=15,
            )

            text = rich_transcription_postprocess(res[0]["text"])
            return text

        except Exception as e:
            print(f"ASR识别过程中发生错误: {e}")
            return ""


    # def recognizer(self, stream_in_audio):
    #     try:
    #         tmpfile = os.path.join(self.output_dir, f"asr-{datetime.now().date()}@{uuid.uuid4().hex}.wav")
    #         self._save_audio_to_file(stream_in_audio, tmpfile)
    #
    #         res = self.model.generate(
    #             input=tmpfile,
    #             cache={},
    #             language="auto",  # 语言选项: "zn", "en", "yue", "ja", "ko", "nospeech"
    #             use_itn=True,
    #             batch_size_s=60,
    #         )
    #
    #         text = rich_transcription_postprocess(res[0]["text"])
    #         (infif"识别文本: {text}")
    #         return text, tmpfile
    #
    #     except Exception as e:
    #         logger.error(f"ASR识别过程中发生错误: {e}")
    #         return None, None

class SileroVAD():
    def __init__(self):
        self.model = load_silero_vad()
        self.sampling_rate = 16000;
        self.threshold = 0.5
        self.min_silence_duration_ms = 200
        self.vad_iterator = VADIterator(self.model,
                            threshold=self.threshold,
                            sampling_rate=self.sampling_rate,
                            min_silence_duration_ms=self.min_silence_duration_ms)
        print(f"VAD Iterator initialized with model {self.model}")

    @staticmethod
    def int2float(sound):
        """
        Convert int16 audio data to float32.
        """
        sound = sound.astype(np.float32) / 32768.0
        return sound

    def is_vad(self, audio_int16:np.ndarray):
        try:
            audio_float32 = self.int2float(audio_int16)
            vad_output = self.vad_iterator(torch.from_numpy(audio_float32))
            if vad_output is not None:
                pass
                print(f"VAD output: {vad_output}")
            return vad_output
        except Exception as e:
            print(f"Error in VAD processing: {e}")
            return None

    def is_vad(self, data:bytes):
        try:
            audio_int16 = np.frombuffer(data, dtype=np.int16)
            audio_float32 = self.int2float(audio_int16)
            vad_output = self.vad_iterator(torch.from_numpy(audio_float32))
            if vad_output is not None:
                pass
                print(f"VAD output: {vad_output}")
            return vad_output
        except Exception as e:
            print(f"Error in VAD processing: {e}")
            return None

    def reset_states(self):
        try:
            self.vad_iterator.reset_states()  # Reset model states after each audio
            print("VAD states reset.")
        except Exception as e:
            print(f"Error resetting VAD states: {e}")

def read_wav_in_chunks(wav_file, chunk_size=1024):
    """
    读取 WAV 文件并按指定大小分割成 chunks
    
    参数:
        wav_file: WAV 文件路径
        chunk_size: 每个 chunk 的大小(字节)，默认为 512
        
    返回:
        生成器，每次产生一个 chunk 的字节数据
    """
    with wave.open(wav_file, 'rb') as wf:
        # 验证参数
        assert wf.getnchannels() == 1, "只支持单声道 WAV 文件"
        assert wf.getsampwidth() == 2, "只支持每个采样点 2 字节的 WAV 文件"
        assert wf.getframerate() == 16000, "只支持 16kHz 采样率的 WAV 文件"
        
        # 计算每个 chunk 包含多少帧(采样点)
        # 每个采样点 2 字节，所以 chunk_size 字节包含 chunk_size/2 个采样点
        frames_per_chunk = chunk_size // 2
        
        # 计算总 chunk 数
        total_frames = wf.getnframes()
        total_chunks = math.ceil(total_frames / frames_per_chunk)
        
        print(f"文件信息: {total_frames} 采样点, {total_frames/16000:.2f} 秒")
        print(f"分割为: {total_chunks} 个 chunks, 每个 {frames_per_chunk} 采样点 ({chunk_size} 字节)")
        
        # 逐个读取 chunk
        for i in range(total_chunks):
            # 定位到当前 chunk 的起始位置
            wf.setpos(i * frames_per_chunk)
            
            # 读取当前 chunk 的帧(采样点)
            # 如果是最后一个 chunk，可能不足 frames_per_chunk
            frames_to_read = min(frames_per_chunk, total_frames - i * frames_per_chunk)
            frames = wf.readframes(frames_to_read)
            
            # 如果最后一个 chunk 不足 512 字节，可以选择填充零或直接返回不足的部分
            # 这里我们直接返回实际数据
            yield frames

if __name__ == "__main__":
    vad = SileroVAD()
    wav_file = sys.argv[1]
    # asr = FunASR();
    # start_time = time.time()
    # asr.recognizer(wav_file)
    # end_time = time.time()
    # print(f"函数执行耗时: {end_time - start_time} 秒")

    for i, chunk in enumerate(read_wav_in_chunks(wav_file)):
        print(i * 512, end=" ")
        if len(chunk) == 1024:
            print(vad.is_vad(chunk))


