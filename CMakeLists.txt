cmake_minimum_required(VERSION 3.10)
project(classicvcom_nova64 VERSION 0.1 LANGUAGES CXX)

find_package(Threads REQUIRED)
find_package(SDL2 REQUIRED)
find_package(fmt REQUIRED)

set(CURRENT_RENDERER "OpenGL" CACHE STRING "Renderer to build with.")
set_property(CACHE CURRENT_RENDERER PROPERTY STRINGS "OpenGL;OpenGL ES 2.0;OpenGL ES 3.0")

if (CURRENT_RENDERER STREQUAL "OpenGL")
	find_package(OpenGL REQUIRED)
	find_package(GLEW REQUIRED)
	set(CURRENT_RENDERER_SOURCE_DIR "opengl")
	set(CURRENT_RENDERER_INCLUDE_DIR "opengl")
	set(CURRENT_RENDERER_LIBRARIES OpenGL::GL GLEW::glew)
elseif (CURRENT_RENDERER STREQUAL "OpenGL ES 2.0")
	find_package(OpenGL REQUIRED)
	set(CURRENT_RENDERER_SOURCE_DIR "opengles2")
	set(CURRENT_RENDERER_INCLUDE_DIR "opengles2")
	set(CURRENT_RENDERER_LIBRARIES OpenGL::GL)
elseif (CURRENT_RENDERER STREQUAL "OpenGL ES 3.0")
	find_package(OpenGL REQUIRED)
	set(CURRENT_RENDERER_SOURCE_DIR "opengles3")
	set(CURRENT_RENDERER_INCLUDE_DIR "opengles3")
	set(CURRENT_RENDERER_LIBRARIES OpenGL::GL)
endif ()

set(CHIPSET_SOURCE_FILES
	src/gpu.cpp
	src/audio.cpp
	src/input.cpp
	src/timer.cpp
	src/network.cpp
	src/floating_point.cpp
	src/dma.cpp)

add_executable(classicvcom_nova64 src/${CURRENT_RENDERER_SOURCE_DIR}/renderer.cpp ${CHIPSET_SOURCE_FILES} src/instructions.cpp src/motherboard.cpp src/cpu.cpp src/os/os.cpp src/main.cpp)
target_include_directories(classicvcom_nova64 PUBLIC "${PROJECT_SOURCE_DIR}/include" "${SDL2_INCLUDE_DIRS}" "${PROJECT_SOURCE_DIR}/include/${CURRENT_RENDERER_INCLUDE_DIR}")
target_compile_features(classicvcom_nova64 PRIVATE cxx_std_20)
target_link_libraries(classicvcom_nova64 Threads::Threads SDL2 ${CURRENT_RENDERER_LIBRARIES} fmt::fmt msbtfont)
