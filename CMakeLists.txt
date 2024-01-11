cmake_minimum_required(VERSION 3.20)
project(metal_cmake_example)

set(CMAKE_CXX_STANDARD 17)

# Choose metal-cpp version
if (NOT METAL_CPP_VERSION)
    set(METAL_CPP_VERSION "macOS14.2_iOS17.2")
endif ()

# Set up cmake build directories to ./build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)


# Download and build metal-cpp if it doesn't exist.
if (NOT EXISTS ./metal-cpp.zip)
    execute_process(
            COMMAND curl -o metal-cpp.zip https://developer.apple.com/metal/cpp/files/metal-cpp_${METAL_CPP_VERSION}.zip
    )
endif ()

if (NOT EXISTS ./metal-cpp)
    execute_process(
            COMMAND unzip metal-cpp.zip
            WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif ()

# *.metal files are compiled to *.metallib files
file(GLOB METAL_SHADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.metal)

# When metal shader files are changed, recompile them.
add_custom_command(
        OUTPUT ./metal_build/shaders.air
        COMMAND xcrun -sdk macosx metal -c ${METAL_SHADER_FILES} -o ./metal_build/shaders.air
        DEPENDS ${METAL_SHADER_FILES}
        VERBATIM
)

add_custom_command(
        OUTPUT ./metal_build/shaders.metallib
        COMMAND xcrun -sdk macosx metallib ./metal_build/shaders.air -o ./metal_build/shaders.metallib
        DEPENDS ./metal_build/shaders.air
        VERBATIM
)

add_custom_target(
        metal_shader_files ALL
        DEPENDS ./metal_build/shaders.metallib
)


include_directories(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/metal-cpp)

set(SOURCE_FILES
        main.cpp
        ${METAL_SHADER_FILES}
)
add_executable(metal-cmake-example ${SOURCE_FILES})

add_dependencies(metal-cmake-example metal_shader_files)

find_library(METAL Metal)
find_library(FOUNDATION Foundation)
find_library(QUARTZCORE QuartzCore)

target_link_libraries(metal-cmake-example
        stdc++
        "-framework Metal"
        "-framework Foundation"
        "-framework QuartzCore"
        objc)
