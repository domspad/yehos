#! /bin/bash

# run with "sudo"

# set up environment variables
INSTALL_PATH=/opt/cross
export PATH=$INSTALL_PATH/bin:$PATH
TARGET=i386-elf32
# export PATH=/usr/local/gcc-6.3.0/bin:$PATH (because I have another compiler...)

# get binutils, gcc, mpfr, gmp, mpc, and isl, and newlib for our C standard library implementation (mac friendly)
wget http://ftpmirror.gnu.org/binutils/binutils-2.28.tar.gz
wget http://ftpmirror.gnu.org/gcc/gcc-6.3.0/gcc-6.3.0.tar.gz
wget http://ftpmirror.gnu.org/mpfr/mpfr-3.1.5.tar.xz
wget http://ftpmirror.gnu.org/gmp/gmp-6.1.2.tar.xz
wget http://ftpmirror.gnu.org/mpc/mpc-1.0.3.tar.gz
wget ftp://gcc.gnu.org/pub/gcc/infrastructure/isl-0.16.1.tar.bz2
wget -nc -O newlib-master.zip https://github.com/bminor/newlib/archive/master.zip || true
unzip -qo newlib-master.zip

# preparing the gcc compilation by linking the libraries into its build directory
tar -xfk gcc-6.3.0.tar.gz
tar -xfk mpfr-3.1.5.tar.xz
tar -xfk gmp-6.1.2.tar.xz
tar -xfk mpc-1.0.3.tar.gz
tar -xvjf isl-0.16.1.tar.bz2
cd gcc-6.3.0
ln -sf `ls -1d ../mpfr-*/` mpfr
ln -sf `ls -1d ../gmp-*/` gmp
ln -sf `ls -1d ../mpc-*/` mpc
ln -sf `ls -1d ../isl-*/` isl
cd ../

# building binutils...
tar -xfk binutils-2.28.tar.gz
mkdir -p build-binutils        # when building, always use a directory different from the source
cd build-binutils/
../binutils-2.28/configure --prefix=$INSTALL_PATH --target=$TARGET --disable-multilib 
make -j4
make install
cd ../

# begin building gcc...
mkdir -p build-gcc
cd build-gcc
../gcc-6.3.0/configure --prefix=/opt/cross --target=$TARGET --enable-languages=c --with-newlib --disable-multilib
make -j4 all-gcc
make install-gcc
cd ../

# building newlib...
mkdir -p build-newlib
cd build-newlib
../newlib-master/configure --prefix=$INSTALL_PATH --target=$TARGET --disable-multilib
make -j4
make install
cd ../

#build the rest of GCC
cd build-gcc
make -j4 all
make install
cd ../

echo 'DONE'

