#! /bin/sh

cmake -DCMAKE_PREFIX_PATH=`python -c 'import torch;print(torch.utils.cmake_prefix_path)'` -DCMAKE_CXX_FLAGS=-pg .
cmake -S . -B out/build