# Yggdrasil build_tarballs.jl recipe for qwtw_jll v3.0.0 (Qt6, zero Boost/Marble)
# Place at `Q/qwtw/build_tarballs.jl` in a PR to https://github.com/JuliaPackaging/Yggdrasil

using BinaryBuilder, Pkg

const YGGDRASIL_DIR = "../.."
include(joinpath(YGGDRASIL_DIR, "platforms", "macos_sdks.jl"))

name = "qwtw"
version = v"3.0.0"

sources = [
    GitSource("https://github.com/ig-or/qwtw.git",
              "REPLACE_WITH_QWTW_COMMIT_SHA"),
    GitSource("https://github.com/ig-or/qwt.git",
              "2dc09fd351ec8c30252913b5eb8af6c46ef4101c"),
]

script = raw"""
# Use newer CMake from HostBuildDependency("CMake_jll")
apk del cmake

# Populate vendored Qwt submodule directory from the second GitSource
rm -rf $WORKSPACE/srcdir/qwtw/extern/qwt
mv $WORKSPACE/srcdir/qwt $WORKSPACE/srcdir/qwtw/extern/qwt

cd $WORKSPACE/srcdir/qwtw
mkdir build && cd build

CMAKE_EXTRA_ARGS=()
if [[ "${target}" == *apple-darwin* ]]; then
    CMAKE_EXTRA_ARGS+=(
        -DQT_HOST_PATH=${host_prefix}
        -DCMAKE_PREFIX_PATH=${host_prefix}
        -DQT_NO_APPLE_SDK_AND_XCODE_CHECK=ON
    )
else
    CMAKE_EXTRA_ARGS+=(
        -DQT_HOST_PATH=${host_prefix}
    )
fi

cmake -G Ninja \
    -DCMAKE_INSTALL_PREFIX=${prefix} \
    -DCMAKE_FIND_ROOT_PATH=${prefix} \
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TARGET_TOOLCHAIN} \
    -DCMAKE_BUILD_TYPE=Release \
    -DNO_VERSION_INFO=ON \
    -DWITH_UDP_SYNC=ON \
    -DUSE_MGL=OFF \
    "${CMAKE_EXTRA_ARGS[@]}" \
    ..

cmake --build . --parallel ${nproc}
cmake --install .

install_license $WORKSPACE/srcdir/qwtw/LICENSE
"""

# MacOSX14.0 SDK is required by Qt6 >= 6.8 and C++23 std::filesystem on macOS
sources, script = require_macos_sdk("14.0", sources, script; deployment_target="12")

platforms = [
    Platform("x86_64",  "linux"; libc="glibc"),
    Platform("aarch64", "linux"; libc="glibc"),
    Platform("x86_64",  "macos"),
    Platform("aarch64", "macos"),
    Platform("x86_64",  "windows"),
]
platforms = expand_cxxstring_abis(platforms)

products = [
    LibraryProduct("libqwtw", :qwtw),
    ExecutableProduct("qwproc", :qwproc),
]

dependencies = [
    HostBuildDependency("CMake_jll"),
    HostBuildDependency("Qt6Base_jll"),
    Dependency("Qt6Base_jll"; compat="~6.10.2"),
    Dependency("CompilerSupportLibraries_jll"),
    BuildDependency("Vulkan_Headers_jll"),
]

# GCC 13+ provides full C++23 support and avoids GCC <= 12 ICEs with Qt6 on MinGW;
# LLVM 16 matches Yggdrasil's Qt6Base build toolchain.
build_tarballs(ARGS, name, version, sources, script, platforms, products, dependencies;
               julia_compat="1.6",
               preferred_gcc_version=v"13",
               preferred_llvm_version=v"16.0.6")
