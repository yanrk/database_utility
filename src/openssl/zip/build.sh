tar -xzf openssl-1.1.1c.tar.gz
cd openssl-1.1.1c
./config --prefix=$HOME/gnu_libs/openssl no-shared
make
mkdir -p ../../lib/linux
cp lib*.a ../../lib/linux
