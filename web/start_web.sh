echo "open http://localhost:6003 to explore"
dir=`dirname $0`
cd $dir
python3 -m http.server 6003
