Simple 3D Rendering Program
=====================

Libraries in use:

* [SFML](https://www.sfml-dev.org/) - For window management and input handling
* [Vulkan](https://www.vulkan.org/) - For GPU acceleration and rendering
* [Slang](https://shader-slang.org) - For GPU shader programming

Dependencies are automatically detected if they already are installed or downloaded
using [CPM](https://github.com/cpm-cmake/CPM.cmake) if they cannot be found

Building
--------
Requires CMake 3.25 or higher

```bash
cmake -B build
cmake --build build --config Release
```