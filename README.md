# Prerequisites

1. [Vulkan SDK](https://vulkan.lunarg.com/#new_tab) added to `PATH` variable.
2. **Visual Studio 2022** or **clang++** compiler with `c++23` support.
3. **CMake** version `3.19` and higher.

<br>

# Configuration
```bash
git clone https://github.com/apetor56/vulkan-engine.git
cd vulkan-engine
```

**1. Build Visual Studio 2022 solution**

Navigate to root directory and run `setup.bat` script to generate Visual Studio solution:
```bash
./setup.bat
```

Generated solution project is located in `vulkan-engine/build/msvc` directory.

**2. Clang++ with MinGW Makefiles/Ninja generator:**

Generate **MinGW Makefiles** and build executable with predefined CMake presets (debug version):
```bash
cmake --preset clang-mingw-debug
cmake --build --preset clang-mingw-debug
```

Equivalent version for **Ninja** generator:
```bash
cmake --preset clang-ninja-debug
cmake --build --preset clang-ninja-debug
```

Built executable path:
```bash
vulkan-engine/build/clang/debug/source/VulkanEngine.exe
```
