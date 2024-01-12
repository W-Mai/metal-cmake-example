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
add_custom_command(
        OUTPUT ./metal-cpp.zip
        COMMAND curl -o metal-cpp.zip https://developer.apple.com/metal/cpp/files/metal-cpp_${METAL_CPP_VERSION}.zip
)

add_custom_command(
        OUTPUT ./metal-cpp/*
        COMMAND unzip metal-cpp.zip
        DEPENDS ./metal-cpp.zip
)

add_custom_target(
        metal_cpp ALL
        DEPENDS ./metal-cpp ./metal-cpp.zip
)

# *.metal files are compiled to *.metallib files
file(GLOB METAL_SHADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.metal)
list(LENGTH METAL_SHADER_FILES METAL_SHADER_FILES_COUNT)

set(AIR_SHADER_FILES "")
foreach(METAL_SHADER_FILE ${METAL_SHADER_FILES})
    get_filename_component(SHADER_NAME ${METAL_SHADER_FILE} NAME_WE)
    set(AIR_SHADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/metal_build/${SHADER_NAME}.air")
    list(APPEND AIR_SHADER_FILES ${AIR_SHADER_FILE})
endforeach()

# When metal shader files are changed, recompile them.
foreach(AIR_SHADER_FILE ${AIR_SHADER_FILES})
    get_filename_component(AIR_SHADER_FILE_NAME ${AIR_SHADER_FILE} NAME_WE)
    add_custom_command(
            OUTPUT ${AIR_SHADER_FILE}
            COMMAND xcrun -sdk macosx metal -c ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${AIR_SHADER_FILE_NAME}.metal -o ${AIR_SHADER_FILE}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${AIR_SHADER_FILE_NAME}.metal
    )
endforeach()

add_custom_command(
        OUTPUT ./metal_build/shaders.metallib
        COMMAND xcrun -sdk macosx metallib ${AIR_SHADER_FILES} -o ./metal_build/shaders.metallib
        DEPENDS ${AIR_SHADER_FILES}
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

add_dependencies(metal-cmake-example
        metal_cpp
        metal_shader_files
)

find_library(METAL Metal)
find_library(FOUNDATION Foundation)
find_library(QUARTZCORE QuartzCore)

target_link_libraries(metal-cmake-example
        "-framework Metal"
        "-framework Foundation"
        "-framework QuartzCore"
        objc)
