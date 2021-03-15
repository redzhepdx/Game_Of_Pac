#! /bin/sh

# export Torch_DIR=/absolute/path/to/site-packages/torch/share/cmake/Torch
mkdir -p out/build
cmake -D CMAKE_BUILD_TYPE=Release  -S . -B out/build
# cmake -S . -B out/build