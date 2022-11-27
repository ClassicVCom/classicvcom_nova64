# ClassicVCom Nova64
A next generation virtual (or fantasy) computer utilizing a hybrid 64-bit custom ISA compared to it's unreleased predecessor the ClassicVCom HD.

## Important Note
This is a very early build under heavy development.  Changes are bound to happen as a result.  There is a test program hardcoded for testing purposes.

## Currently Supported Renderers
- OpenGL 3.3
- OpenGL ES 3.0 (Compatible with Raspberry Pi 4 and other devices.)

## Future Renderer Support
- OpenGL ES 2.0
- Vulkan
- Direct3D 11 (Targeting Direct3D 10 class hardware.)
- Direct3D 12

## Requirements for Building
- [CMake](https://www.cmake.org/download/) (at least 3.10)
- [fmt](https://github.com/fmt)
- [libmsbtfont](https://github.com/Bandock/libmsbtfont) (Requires at least 0.2.0, latest release recommended)
- [SDL2](https://www.libsdl.org/download-2.0.php) (Latest stable development version should work fine)
- [GLEW](http://glew.sourceforge.net) (If you're compiling with OpenGL 3.3 renderer support)
- C++ Compiler with C++20 Support
