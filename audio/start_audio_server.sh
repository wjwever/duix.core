dir=`dirname $0`
cd $dir
sudo apt update
sudo apt install python3  python3-pip ffmpeg
pip3 install -r requirements.txt   -i https://pypi.doubanio.com/simple/
echo "download SenseVoiceSmall from huggingface"
export HF_ENDPOINT="https://hf-mirror.com"

bash ./hfd.sh	FunAudioLLM/SenseVoiceSmall

