#!/usr/bin/env bash
set -euo pipefail

cd /workspaces/id3dev
# mkdir -p /root/.cache/ccache

# Seed a GCC + GNU Make configure and emit compile_commands.json for clangd.
#if [[ -f CMakeLists.txt ]]; then
#  cmake -S . -B cmake-build-debug -G "Unix Makefiles" \
#    -DCMAKE_C_COMPILER=gcc \
#    -DCMAKE_CXX_COMPILER=g++ \
#    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
#    -DDEBUG_ID3_SYMBOLS=ON \
#    -DBUILD_ID3_TESTS=ON \
#    -DBUILD_ID3_C_EXAMPLES=ON \
#    -DBUILD_ID3_DOCS=OFF
#fi
