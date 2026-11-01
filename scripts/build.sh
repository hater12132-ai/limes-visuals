#!/bin/bash
set -euo pipefail

NDK="\( {1:- \){ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-}}}"

if [[ -z "$NDK" || ! -d "$NDK" ]]; then
    echo "Error: Android NDK not found."
    echo "Pass it as argument or set ANDROID_NDK_HOME / ANDROID_NDK_ROOT"
    exit 1
fi

ABI="arm64-v8a"
BUILD_DIR="build-${ABI}"

echo "==> Configuring for ${ABI} ..."
cmake -S . -B "${BUILD_DIR}" \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="${NDK}/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="${ABI}" \
    -DANDROID_PLATFORM=android-28 \
    -DANDROID_STL=c++_shared \
    -DCMAKE_BUILD_TYPE=Release

echo "==> Building + packaging ..."
cmake --build "\( {BUILD_DIR}" --target package_levipack -j" \)(nproc 2>/dev/null || echo 4)"

echo ""
echo "Done! Output:"
ls -lh "${BUILD_DIR}/limes-visuals.levipack"
