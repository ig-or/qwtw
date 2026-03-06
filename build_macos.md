# Building qwtw on macOS

## Prerequisites

Install the following via Homebrew:

```bash
brew install cmake ninja boost qt
```

Qwt is built from the vendored submodule at `extern/qwt` — no separate install needed.

### Verify Qt6 is visible

```bash
brew --prefix qt
# Should print something like /opt/homebrew/opt/qt
```

The build system auto-discovers Homebrew Qt6 from `/opt/homebrew/opt/qt`.

## Clone (with submodules)

```bash
git clone --recurse-submodules <repo-url>
cd qwtw
```

If you already cloned without `--recurse-submodules`:

```bash
git submodule update --init --recursive
```

## Build

```bash
mkdir -p build_mac_qt6 && cd build_mac_qt6
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
```

### Optional features

| Flag | Default | Description |
|------|---------|-------------|
| `-DWITH_UDP_SYNC=ON` | OFF | UDP marker position broadcast between instances |
| `-DUSE_MARBLE=ON` | OFF | Marble-qt map/top-view support |
| `-DUSE_MGL=ON` | OFF | MathGL 3-D plotting |

Example with all options:

```bash
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DWITH_UDP_SYNC=ON -DUSE_MARBLE=ON -DUSE_MGL=ON
```

## Output

Binaries are placed in `lib/macos-arm64/release/` (or `lib/macos-x86_64/release/` on Intel):

| File | Description |
|------|-------------|
| `libqwtw.dylib` | Main shared library |
| `qwproc` | Plot server process |
| `qwtest2` | Test executable (dynamically loads `libqwtw.dylib`) |
| `qttest` | Qt integration test |

## Run the test

```bash
cd lib/macos-arm64/release
./qwtest2
```

`qwtest2` finds `libqwtw.dylib` next to itself automatically. Press M to
activate the marker/picker, then drag across a plot — markers on all figures
should follow.

## Troubleshooting

### CMake can't find Qt6

Set the prefix explicitly:

```bash
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
```

### Rebuild from scratch

```bash
rm -rf CMakeCache.txt CMakeFiles
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
```
