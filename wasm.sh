#!/bin/bash
mkdir -p build-wasm && cd build-wasm
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
emmake make
sudo make install  # Uses the custom WASM install path
