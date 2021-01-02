cmake_minimum_required (VERSION 2.8.11)
project(WAVES C)
find_library(m m)
find_library(GLESv2 GLESv2)
find_library(GLEW GLEW)
find_library(glfw glfw)

if (NOT WIN32)
    add_compile_options("-Wall" "-Wpedantic"  "-g" "-O0")
endif()

add_executable(waves
	           ${PROJECT_SOURCE_DIR}/read_file.c
               ${PROJECT_SOURCE_DIR}/gl_wrappers.c
               ${PROJECT_SOURCE_DIR}/waves.c
               )
target_link_libraries(waves m GLESv2 GLEW glfw)

