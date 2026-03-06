# Building qwtw on Linux

## Option A: Native build on Ubuntu/Debian

### Prerequisites

```bash
sudo apt-get update && sudo apt-get install -y \
  build-essential cmake ninja-build pkg-config \
  qt6-base-dev qt6-tools-dev libqt6opengl6-dev libqt6openglwidgets6 \
  libboost-all-dev libssl-dev \
  libgl-dev
```

Qwt is built from the vendored submodule at `extern/qwt` — no separate install needed.

### Clone (with submodules)

```bash
git clone --recurse-submodules <repo-url>
cd qwtw
```

If you already cloned without `--recurse-submodules`:

```bash
git submodule update --init --recursive
```

### Build

```bash
mkdir -p build_linux && cd build_linux
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
```

### Optional features

| Flag | Default | Description |
|------|---------|-------------|
| `-DWITH_UDP_SYNC=ON` | OFF | UDP marker position broadcast between instances |
| `-DUSE_MARBLE=ON` | OFF | Marble-qt map/top-view support |
| `-DUSE_MGL=ON` | OFF | MathGL 3-D plotting |

### Output

Binaries are placed in `lib/linux-aarch64/release/` (ARM64) or
`lib/linux-x86_64/release/` (x86-64):

| File | Description |
|------|-------------|
| `libqwtw.so` | Main shared library |
| `qwproc` | Plot server process |
| `qwtest2` | Test executable (dynamically loads `libqwtw.so`) |
| `qttest` | Qt integration test |

### Run the test

```bash
cd lib/linux-aarch64/release   # adjust for your architecture
./qwtest2
```

---

## Option B: Docker container (build on macOS, run Linux)

This is the recommended approach when developing on macOS with a shared
filesystem (e.g. OneDrive). macOS and Linux binaries are placed in separate
platform-specific directories and do not conflict.

### Prerequisites on the Mac host

1. Docker Desktop installed and running.
2. XQuartz installed (for GUI forwarding):

```bash
brew install --cask xquartz
```

3. After installing XQuartz, **log out and log back in**.
4. Allow X11 connections (run each session, or add to `~/.zshrc`):

```bash
/opt/X11/bin/xhost +localhost
```

### Build the Docker image

The Dockerfile is at `space/dc/base/Dockerfile`. It includes Qt6, Boost,
and all build dependencies.

```bash
# Copy the Julia tarball into the build context if needed:
cp ~/Downloads/julia-1.12.5-linux-aarch64.tar /path/to/space/dc/base/

# Build the image:
docker build --platform linux/arm64 \
  -t work:ubuntu24-arm /path/to/space/dc/base/
```

### Start the container

```bash
docker compose -f "/path/to/space/dc/composer.yaml" run --rm aenv
```

The container mounts the workspace at `/space` and sets
`DISPLAY=host.docker.internal:0` for GUI forwarding.

### Build inside the container

```bash
cd /space/qwtw
mkdir -p build_linux && cd build_linux
CC=/usr/bin/cc CXX=/usr/bin/c++ cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
```

> **Note:** The `CC`/`CXX` override is needed on the first run because the
> container has ccache wrappers in `PATH` that can confuse CMake. After the
> initial configure, plain `ninja` is sufficient for rebuilds.

### Run the test (with GUI)

Make sure XQuartz is running and `xhost +localhost` was executed on the Mac,
then inside the container:

```bash
cd /space/qwtw/lib/linux-aarch64/release
./qwtest2
```

Plot windows will appear on your Mac via X11 forwarding.

## Troubleshooting

### `ccache` compiler errors during cmake configure

```bash
rm -rf CMakeCache.txt CMakeFiles
CC=/usr/bin/cc CXX=/usr/bin/c++ cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### `qt.qpa.xcb: could not connect to display`

1. Verify XQuartz is running on the Mac: `ps aux | grep Xquartz`
2. Verify it listens on TCP (should show `-listen tcp`, not `-nolisten tcp`):
   ```bash
   defaults write org.xquartz.X11 nolisten_tcp -bool false
   ```
   Then restart XQuartz and re-run `xhost +localhost`.
3. Verify port 6000 is open: `lsof -i TCP:6000`
4. Inside the container, confirm: `echo $DISPLAY` should show
   `host.docker.internal:0`.

### Exec format error

You are running a binary built for a different platform (e.g. macOS Mach-O
binary on Linux). Each platform outputs to its own directory under `lib/`:

- macOS ARM64: `lib/macos-arm64/release/`
- Linux ARM64: `lib/linux-aarch64/release/`
- Linux x86-64: `lib/linux-x86_64/release/`
