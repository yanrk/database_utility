# build hiredis
unzip hiredis-1.1.0.zip
cd hiredis-1.1.0
mkdir cmake-build
cd cmake-build
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_SSL=ON -DOPENSSL_ROOT_DIR=/home/yanrenke/prefix/openssl ..
make
