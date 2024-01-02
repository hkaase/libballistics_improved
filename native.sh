#!/bin/bash
mkdir -p build-native && cd build-native
cmake ..
make
sudo make install
