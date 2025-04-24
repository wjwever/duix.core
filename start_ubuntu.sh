echo "make"
sudo apt update
sudo apt install build-essential cmake  wget curl  ffmpeg git libopencv-dev libcurl4-openssl-dev
#git clone --recurse-submodules https://github.com/wjwever/duix.ai.core.git
#cd duix.ai.core
mkdir -p build; cd build; cmake ../; make -j 4
cp ../conf . -r

 
echo "download resource"
wget https://cdn.guiji.ai/duix/location/gj_dh_res.zip
unzip gj_dh_res.zip
mkdir -p roles
wget https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/digital/model/1719194450521/siyao_20240418.zip
unzip siyao_20240418.zip
mv siyao_20240418 roles/SiYao

wget https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/digital/model/1719194007931/bendi1_0329.zip
unzip bendi1_0329.zip
mv bendi1_0329 roles/DearSister

./ws_server
