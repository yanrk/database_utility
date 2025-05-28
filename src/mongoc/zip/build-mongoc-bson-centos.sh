
# build release on centos

# install cmake 3
yum info cmake
yum remove cmake -y
wget https://cmake.org/files/v3.6/cmake-3.6.2.tar.gz
tar -zxvf cmake-3.6.2.tar.gz
cd cmake-3.6.2
./bootstrap --prefix=/usr/local
make
make install
ln -s /usr/local/bin/cmake /usr/bin/cmake
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
