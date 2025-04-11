sudo apt install build-essential cmake  wget curl  ffmpeg git libopencv-dev libcurl4-openssl-dev
git clone --recurse-submodules https://github.com/wjwever/duix.core.git
cd duix.core
mkdir -p build
cd build
cmake ../
make -j
cp ../conf . -r
