# Gachix - simple vulkan renderer

This project is a graphics renderer built with the **Vulkan SDK** as part of a Master's thesis. The main objective was to implement indirect **Physically Based Rendering (PBR)**, while also achieving a solid understanding of core Vulkan mechanisms. This included low-level setup of graphics pipelines, resource management (such as explicit control over memory and buffers), synchronization primitives, offscreen rendering, and postprocessing effects like anti-aliasing.

## Screens
### Indirect PBR

<img width="2560" height="1440" alt="scene_9" src="https://github.com/user-attachments/assets/3af2fb59-7ac7-4806-ae5c-be08bfac175c" />

### IBL - diffuse
<img width="2560" height="1369" alt="1" src="https://github.com/user-attachments/assets/5720a36e-a2bb-4a66-9e6c-b74744b15588" />
<img width="2560" height="1369" alt="2" src="https://github.com/user-attachments/assets/843dec4f-bf3f-4026-b3b7-2598b663b005" />

### IBL - specular (in progress)

## Prerequisites

1. [Vulkan SDK](https://vulkan.lunarg.com/#new_tab) added to `PATH` variable.
2. **Visual Studio 2022** or **clang++** compiler with `c++23` support.
3. **CMake** version `3.19` and higher.

<br>

## Configuration
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

## Used dependencies:
- [GLFW](https://github.com/glfw/glfw)
- [GLM](https://github.com/g-truc/glm)
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- [fastgltf](https://github.com/spnda/fastgltf)
- [spdlog](https://github.com/gabime/spdlog)
- [stb](https://github.com/nothings/stb)
