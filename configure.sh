#! /bin/sh

# export Torch_DIR=/absolute/path/to/site-packages/torch/share/cmake/Torch
mkdir -p out/build
cmake -DCMAKE_BUILD_TYPE=Release out/build
cmake -S . -B out/build