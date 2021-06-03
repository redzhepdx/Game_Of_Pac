#! /bin/sh

# export Torch_DIR=/absolute/path/to/site-packages/torch/share/cmake/Torch
# export CC=/usr/bin/clang-6.0
# export CXX=/usr/bin/clang++-6.0
mkdir -p out/build
cmake -D CMAKE_BUILD_TYPE=Release -S . -B out/build
# cmake -S . -B out/build
