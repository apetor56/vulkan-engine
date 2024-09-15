# Prerequisites

1. [Vulkan SDK](https://vulkan.lunarg.com/#new_tab) added to `PATH` variable.
2. **Visual Studio 2022** or **clang++** compiler with `c++23` support.
3. **CMake** version `3.19` and higher.

<br>

# Configuration
### Windows
**1. Build Visual Studio 2022 solution:**

Navigate to root directory and run `setup.bat` script to generate Visual Studio solution:
```bash
cd vulkan-engine
./setup.bat
```

Generated solution project is located in `vulkan-engine/build/msvc` directory.

**2. Clang++ with MinGW Makefiles:**

Generate **MinGW Makefiles** and build executable with predefined CMake presets (debug version):
```bash
cd vulkan-engine
cmake --preset clang-debug-windows
cmake --build --preset clang-debug-windows
```

Equivalent version for release:
```bash
cd vulkan-engine
cmake --preset clang-release-windows
cmake --build --preset clang-release-windows
```

---

### Linux (experimental)
**Clang++ with Unix Makefiles**

Generate **Unix Makefiles** and build executable with predefined CMake presets (debug version):
```bash
cd vulkan-engine
cmake --preset clang-debug-linux
cmake --build --preset clang-debug-linux
```

Equivalent version for release:
```bash
cd vulkan-engine
cmake --preset clang-release-linux
cmake --build --preset clang-release-linux
```
---
For clang++ and Makefile option executable is located in `vulkan-engine\build\clang\debug\source` directory (or in `vulkan-engine\build\clang\release\source` for release version).
