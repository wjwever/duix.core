echo "open http://localhost:9000 to explore"
dir=`dirname $0`
cd $dir
python3 -m http.server 9000
