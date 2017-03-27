cmake_minimum_required(VERSION 3.2)

set(APP_NAME gl_boilerplate)

set(SOURCES src/main.c)

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

find_package(OpenGL REQUIRED)

# header only stuff

include_directories(thirdparty)

##

add_library(gl_utils STATIC
                     lib/src/gl_utils/gl_helpers.c
                     lib/include/gl_utils)
include_directories(lib/include)

add_executable(${APP_NAME} ${SOURCES})
target_link_libraries(${APP_NAME} ${OPENGL_LIBRARIES} glfw glad gl_utils)
