#!/bin/bash

wget "https://github.com/y-256/libdivsufsort/archive/master.zip"
unzip master && rm master*

cd libdivsufsort-master
mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE="Release" \
    -DCMAKE_INSTALL_PREFIX="." \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_DIVSUFSORT64=ON \
    -DUSE_OPENMP=ON \
    ..
make install