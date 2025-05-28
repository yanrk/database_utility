
# build release on ubuntu

# install cmake 3
sudo apt-get install cmake
cmake --version

# install openssl
tar -xzf openssl-1.1.1c.tar.gz
cd openssl-1.1.1c
./config -fPIC --prefix=/home/yanrenke/prefix/openssl no-shared
make
make install

# build mongo-c-driver-1.23.2
tar -xzf mongo-c-driver-1.23.2.tar.gz
cd mongo-c-driver-1.23.2
mkdir cmake-build
cd cmake-build
cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DCMAKE_BUILD_TYPE=Release -DENABLE_STATIC=ON .. -DOPENSSL_ROOT_DIR=/home/yanrenke/prefix/openssl -DENABLE_EXAMPLES=OFF -DENABLE_TEST=OFF
make
# make with 4 thread: make -j4 
# make install

