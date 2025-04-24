//  get document element
const messagesDiv = document.getElementById('messages');
const audioInput = document.getElementById('audioInput');

const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');
const audioInfo = document.getElementById('audioInfo');
const loadingIndicator = document.getElementById('loading');

// microphone
let audioWs;
let listen = true;
let mediaStream;
let audioContext;
let processor;
let interval;
let audioChunks = [];
const SEND_INTERVAL = 200; // ms

// image 
let ws;
const width = 540;
const height = 960;

// fps count
let frameCount = 0;
let lastFpsUpdate = 0;
let fps = 0;
const fpsUpdateInterval = 1000; // 1秒更新一次FPS

// audio play
let audioPlayer = new Audio("");
let isCalling = false; // 当前通信状态

let localVideoStream;

  
// get audio devices
async function getAudioDevices() {
  try {
    audioInput.disabled = true;
    audioInput.innerHTML = '<option value="">-- Loading devices --</option>';

    // 需要先获取用户媒体权限才能枚举设备
    await navigator.mediaDevices.getUserMedia({ audio: true });

    const devices = await navigator.mediaDevices.enumerateDevices();
    const audioDevices = devices.filter(device => device.kind === 'audioinput');

    audioInput.innerHTML = '';
    if (audioDevices.length === 0) {
      audioInput.innerHTML = '<option value="">No audio devices found</option>';
      return;
    }

    audioDevices.forEach(device => {
      const option = document.createElement('option');
      option.value = device.deviceId;
      option.text = device.label || `Microphone ${audioDevices.indexOf(device) + 1}`;
      audioInput.appendChild(option);
    });

    audioInput.disabled = false;
  } catch (err) {
    console.error('Error getting audio devices:', err);
    audioInput.innerHTML = '<option value="">Error loading devices</option>';
  }
}
getAudioDevices();
navigator.mediaDevices.addEventListener('devicechange', getAudioDevices);

function open_camera() {
  navigator.getUserMedia(
    { video: true, audio: false },
    stream => {
      const localVideo = document.getElementById("local-video");
      if (localVideo) {
        localVideo.srcObject = stream;
      }
      localVideoStream = stream; // 保存stream引用
    },
    error => {
      console.warn(error.message);
    }
  );

}

// 关闭摄像头的函数
function stop_camera() {
    if (localVideoStream) {
        localVideoStream.getTracks().forEach(track => {
            track.stop(); // 停止每个轨道
        });
        
        // 清除视频元素的srcObject
        const localVideo = document.getElementById("local-video");
        if (localVideo) {
            localVideo.srcObject = null;
        }
        
        localVideoStream = null; // 清除stream引用
    }
}

//  feed audio
async function startCapture() {
    try {
        listen = true;
        open_camera();
        const deviceId = audioInput.value;
        if (!deviceId) {
            alert('Please select a microphone first');
            return;
        }
        const constraints = {
            audio: {
                deviceId: deviceId ? { exact: deviceId } : undefined,
                sampleRate: 16000 // 尝试设置采样率（浏览器可能会忽略）
            },
            video: false
        };

        mediaStream = await navigator.mediaDevices.getUserMedia(constraints);
        audioContext = new (window.AudioContext || window.webkitAudioContext)();
        const source = audioContext.createMediaStreamSource(mediaStream);

        console.log('Using sample rate:', audioContext.sampleRate);

        processor = audioContext.createScriptProcessor(4096, 1, 1);
        source.connect(processor);
        processor.connect(audioContext.destination);

        processor.onaudioprocess = (e) => {
            const audioData = e.inputBuffer.getChannelData(0);
            audioChunks.push(new Float32Array(audioData));
        };

        // 每200ms发送一次音频数据
        interval = setInterval(() => {
            if (audioChunks.length > 0 && audioWs && audioWs.readyState === WebSocket.OPEN) {
                const merged = mergeArrays(audioChunks);
                const int16Data = floatTo16BitPCM(merged);

                // Create metadata object
                const metadata = {
                    event: 'audio',
                    sampleRate: audioContext.sampleRate,
                    timestamp: Date.now(),
                    length: int16Data.length
                };

                // Send as JSON string with binary data
                if (listen) {
                    audioWs.send(JSON.stringify({
                        metadata: metadata,
                        audioData: Array.from(int16Data)
                    }));
                } else {
                    audioWs.send(JSON.stringify({
                        metadata: metadata,
                        audioData: Array.from(int16Data).fill(0)
                    }));
                }

                audioChunks = [];
            }
        }, SEND_INTERVAL);

        audioInput.disabled = true;
        logMessage('Audio capture started with device:' + deviceId);
    } catch (err) {
        errMessage('Error starting capture:' +  err);
        alert('Error starting capture: ' + err.message);
    }
}


// 停止音频采集
function stopCapture() {
    if (mediaStream) {
        mediaStream.getTracks().forEach(track => track.stop());
    }
    if (processor) {
        processor.disconnect();
    }
    if (audioContext) {
        audioContext.close();
    }
    if (interval) {
        clearInterval(interval);
    }
    
    audioInput.disabled = false;
    stop_camera();
    errMessage('Microphone stopped');
}

function toggleCall() {
  const button = document.getElementById('comButton');

  if (!isCalling) {
    startCapture();

    // 切换为挂断状态
    button.textContent = "Leave";
    button.classList.remove('call-state');
    button.classList.add('hangup-state');
    isCalling = true;
  } else {
    stopCapture();

    button.textContent = "Talk";
    button.classList.remove('hangup-state');
    button.classList.add('call-state');
    isCalling = false;
  }
}

//  connet websocket 
function connectWebSocket() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    return
  }
  const wsScheme = window.location.protocol === 'https:' ? 'wss://' : 'ws://';
  const wsUrl = wsScheme + window.location.host + '/ws';

  ws = new WebSocket("ws://localhost:9090");

  ws.onopen = () => {
    lastFpsUpdate = performance.now();
    logMessage("avatar websocket server connected!")
  };

  ws.onclose = () => {
    errMessage("avatar websocket server disconnected!")
  };

  ws.onerror = (error) => {
    console.error('WebSocket error:', error);
    errMessage("avatar websocket server error!");
  };
}

function connectAudioWebSocket() {
    if (audioWs && audioWs.readyState === WebSocket.OPEN) {
        errMessage('audioWs already connected')
        return
    }

    audioWs = new WebSocket("ws://localhost:8765");
    audioWs.onopen = () => {
        logMessage("audio websocket server connected!")
    };
    audioWs.onclose = () => {
        errMessage("audio websocket server disconnected!")
    };
    audioWs.onerror = (error) => {
        errMessage("audio websocket server error!")
    };
    audioWs.onmessage = (event) => {
        logMessage('asr:' + event.data);
        listen = false; 
        logMessage('listen disable')
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(event.data);
        }
    };
}
connectWebSocket();
connectAudioWebSocket();


ws.onmessage = function(event) {
    if (event.data instanceof Blob) {
		const reader = new FileReader();
		reader.onload = function() {
			const data = new Uint8Array(reader.result);

			// 解析JSON长度(前4字节)
			const metadataLength = new DataView(data.buffer).getUint32(0, false);


			// 2. 提取JSON部分（从第4字节开始，长度为jsonLength）
			const jsonBytes = data.subarray(4, 4 + metadataLength);


			// 3. 将UTF-8字节解码为字符串
			const jsonStr = new TextDecoder('utf-8').decode(jsonBytes);

			// 4. 解析JSON对象（自动处理中文）
			const metadata = JSON.parse(jsonStr);

			// 检查是否是音频消息
			if ("wav" in metadata) {
				console.log('play', metadata.wav);
        playAudio(metadata.wav)
          .then(() => logMessage('play:' + metadata.wav))
          .catch(error => errMessage('play error:', error));
			} 
      if ("role" in metadata) {
          logMessage("roles:" + jsonStr);
          updateUserList(metadata.role);
      }
      if ("listen" in metadata) {
        logMessage("listen enable");
        listen = true;
      }

			// 获取图像二进制数据
			const imageData = data.slice(4 + metadataLength);
      if (imageData.length == 0) {
        return;
      }

			// 使用元数据创建ImageData
			const imageDataObj = new ImageData(
				new Uint8ClampedArray(imageData.buffer,
					imageData.byteOffset,
					width * height * 4),
				width,
				height
			);

			// 渲染到canvas
			ctx.putImageData(imageDataObj, 0, 0);
      // 设置文字样式
      ctx.font = '20px Arial';
      ctx.fillStyle = 'white';
      ctx.textBaseline = 'top';
    
      // 添加FPS文字
      ctx.fillText(`FPS:${fps}`, 10, 10);

			// 更新帧率统计
			frameCount++;
			const now = performance.now();
			if (now - lastFpsUpdate >= fpsUpdateInterval) {
				fps = (frameCount * 1000) / (now - lastFpsUpdate);
				frameCount = 0;
				lastFpsUpdate = now;
			}
		};
		reader.readAsArrayBuffer(event.data);
	}
};


// 播放音频函数
function playAudio(audioUrl) {
  // 如果已经有音频在播放，先停止
  if (audioPlayer) {
    stopAudio();
  }

  // 创建新的音频对象
  audioPlayer.src = audioUrl;

  // 设置自动播放（注意：现代浏览器可能会阻止自动播放）
  audioPlayer.autoplay = true;

  // 加载并播放
  audioPlayer.load();

  // 返回Promise以便处理播放状态
  return new Promise((resolve, reject) => {
    audioPlayer.addEventListener('canplaythrough', () => {
      audioPlayer.play()
        .then(() => resolve())
        .catch(error => reject(error));
    });

    audioPlayer.addEventListener('error', (error) => {
      reject(error);
    });
  });
}

// 停止音频（暂停并重置时间）
function stopAudio() {
  if (audioPlayer) {
    audioPlayer.pause();
    audioPlayer.currentTime = 0;
  }
}

// 辅助函数：格式化时间(秒)为MM:SS
function formatTime(seconds) {
    if (!isFinite(seconds)) return '--:--';
    const mins = Math.floor(seconds / 60);
    const secs = Math.floor(seconds % 60);
    return `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
}

// helper functions
function logMessage(message) {
    const messageElement = document.createElement('div');
    messageElement.className = 'status connected';
    messageElement.textContent = message;
    messagesDiv.appendChild(messageElement);
    messagesDiv.scrollTop = messagesDiv.scrollHeight;
}
function errMessage(message) {
    const messageElement = document.createElement('div');
    messageElement.className = 'status disconnected';
    messageElement.textContent = message;
    messagesDiv.appendChild(messageElement);
    messagesDiv.scrollTop = messagesDiv.scrollHeight;
}
function formatDate() {
    const date = new Date();
    const year = date.getFullYear();
    const month = String(date.getMonth() + 1).padStart(2, '0'); // 月份是从0开始的
    const day = String(date.getDate()).padStart(2, '0');
    const hours = String(date.getHours()).padStart(2, '0');
    const minutes = String(date.getMinutes()).padStart(2, '0');
    const seconds = String(date.getSeconds()).padStart(2, '0');
    const milliseconds = String(date.getMilliseconds()).padStart(3, '0'); // 确保毫秒有三位数字
    return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}.${milliseconds}`;
}
function isValidUrl(url) {
    try {
        new URL(url);
        return true;
    } catch {
        return false;
    }
}

function mergeArrays(arrays) {
    let totalLength = arrays.reduce((acc, arr) => acc + arr.length, 0);
    let result = new Float32Array(totalLength);
    let offset = 0;
    for (let arr of arrays) {
        result.set(arr, offset);
        offset += arr.length;
    }
    return result;
}

function floatTo16BitPCM(input) {
    const output = new Int16Array(input.length);
    for (let i = 0; i < input.length; i++) {
        const s = Math.max(-1, Math.min(1, input[i]));
        output[i] = s < 0 ? s * 0x8000 : s * 0x7FFF;
    }
    return output
}

function unselectUsersFromList() {
  const alreadySelectedUser = document.querySelectorAll(
    ".active-user.active-user--selected"
  );

  alreadySelectedUser.forEach(el => {
    el.setAttribute("class", "active-user");
  });
}

function createUserItemContainer(socketId) {
  const userContainerEl = document.createElement("div");

  const usernameEl = document.createElement("p");

  userContainerEl.setAttribute("class", "active-user");
  userContainerEl.setAttribute("id", socketId);
  usernameEl.setAttribute("class", "username");
  usernameEl.innerHTML = `${socketId}`;

  userContainerEl.appendChild(usernameEl);

  userContainerEl.addEventListener("click", () => {
    unselectUsersFromList();
    userContainerEl.setAttribute("class", "active-user active-user--selected");
    callUser(socketId);
  });

  return userContainerEl;
}

function callUser(socketId) {
  logMessage('call ' + socketId);
  ws.send(JSON.stringify({event:"init", role:socketId}));
}

function updateUserList(socketIds) {
  const activeUserContainer = document.getElementById("active-user-container");

  socketIds.forEach(socketId => {
    const alreadyExistingUser = document.getElementById(socketId);
    if (!alreadyExistingUser) {
      const userContainerEl = createUserItemContainer(socketId);

      activeUserContainer.appendChild(userContainerEl);
    }
  });
}
