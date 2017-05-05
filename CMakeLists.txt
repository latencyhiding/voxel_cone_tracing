cmake_minimum_required(VERSION 3.2)

set(APP_NAME Voxel_Cone_Tracing)

set(SOURCES src/main.cpp
            src/renderer.cpp
            src/texture_3d.cpp)

## Third party libs
# glad 

add_library(glad STATIC
                 thirdparty/glad/src/glad.c
                 thirdparty/glad/include
)
include_directories(thirdparty/glad/include)

# GLFW

# Building only the GLFW lib
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")

add_subdirectory(thirdparty/glfw)
include_directories(thirdparty/glfw/include)

find_package(OpenGL REQUIRED)

# GLM
include_directories(thirdparty/glm)

# Imgui
add_library(imgui STATIC
                  thirdparty/imgui/imgui.cpp
                  thirdparty/imgui
                  thirdparty/imgui/imgui.h
                  thirdparty/imgui/imgui_draw.cpp
                  thirdparty/imgui/imgui_demo.cpp
                  thirdparty/imgui/imgui_internal.h
                  thirdparty/imgui/imconfig.h
                  thirdparty/imgui/stb_rect_pack.h
                  thirdparty/imgui/stb_textedit.h
                  thirdparty/imgui/stb_truetype.h
                  thirdparty/imgui/examples/opengl3_example/imgui_impl_glfw_gl3.cpp
                  thirdparty/imgui/examples/opengl3_example/imgui_impl_glfw_gl3.h
                  thirdparty/gl3w/src/gl3w.c)
include_directories(thirdparty/imgui)
include_directories(thirdparty/imgui/examples/opengl3_example)
include_directories(thirdparty/gl3w/include)

# header only stuff
add_library(header_only_impls STATIC 
                              thirdparty/header_only_impls.cpp)
include_directories(thirdparty)

##

add_library(gl_utils STATIC
                     lib/gl_utils/src/gl_helpers.c
                     lib/gl_utils/include)
include_directories(lib/gl_utils/include)

add_executable(${APP_NAME} ${SOURCES})
target_compile_features(${APP_NAME} PRIVATE cxx_range_for)
target_link_libraries(${APP_NAME} ${OPENGL_LIBRARIES} glfw glad gl_utils header_only_impls imgui)
