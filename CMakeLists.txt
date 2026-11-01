cmake_minimum_required(VERSION 3.22)
project(limes_visuals LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MOD_ID              "limes-visuals"          CACHE STRING "Runtime mod id (folder name)")
set(MOD_NAME            "Lime's Visuals"         CACHE STRING "Display name")
set(MOD_AUTHOR          "Lime"                   CACHE STRING "Author")
set(MOD_VERSION         "0.1.0"                  CACHE STRING "Version")
set(MOD_LIBRARY_NAME    "limes_visuals"          CACHE STRING "Output .so name without lib prefix")

include(FetchContent)
FetchContent_Declare(
    preloader
    GIT_REPOSITORY https://github.com/LiteLDev/preloader-android.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(preloader)

add_library(${MOD_LIBRARY_NAME} SHARED
    src/LimesVisuals.cpp
    src/Chams.cpp
)

target_include_directories(${MOD_LIBRARY_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${preloader_SOURCE_DIR}/include
)

target_link_libraries(${MOD_LIBRARY_NAME} PRIVATE
    log
    android
    EGL
    GLESv3
)

if(ANDROID)
    target_compile_definitions(${MOD_LIBRARY_NAME} PRIVATE ANDROID)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${MOD_LIBRARY_NAME} PRIVATE -O2 -fvisibility=hidden)
    target_link_options(${MOD_LIBRARY_NAME} PRIVATE -s)
endif()

add_custom_target(package_levipack
    COMMAND \( {CMAKE_COMMAND} -E make_directory " \){CMAKE_BINARY_DIR}/package/${MOD_ID}"
    COMMAND \( {CMAKE_COMMAND} -E copy " \)<TARGET_FILE:\( {MOD_LIBRARY_NAME}>" " \){CMAKE_BINARY_DIR}/package/\( {MOD_ID}/lib \){MOD_LIBRARY_NAME}.so"
    COMMAND \( {CMAKE_COMMAND} -E copy " \){CMAKE_SOURCE_DIR}/manifest.json" "\( {CMAKE_BINARY_DIR}/package/ \){MOD_ID}/manifest.json"
    COMMAND \( {CMAKE_COMMAND} -E make_directory " \){CMAKE_BINARY_DIR}/package/${MOD_ID}/config"
    COMMAND \( {CMAKE_COMMAND} -E copy_if_different " \){CMAKE_SOURCE_DIR}/config/config.json" "\( {CMAKE_BINARY_DIR}/package/ \){MOD_ID}/config/config.json"
    COMMAND \( {CMAKE_COMMAND} -E copy_if_different " \){CMAKE_SOURCE_DIR}/config/config.schema.json" "\( {CMAKE_BINARY_DIR}/package/ \){MOD_ID}/config/config.schema.json"
    COMMAND \( {CMAKE_COMMAND} -E chdir " \){CMAKE_BINARY_DIR}/package" \( {CMAKE_COMMAND} -E tar "cf" " \){CMAKE_BINARY_DIR}/\( {MOD_ID}.levipack" --format=zip " \){MOD_ID}"
    COMMENT "Packaging ${MOD_ID}.levipack"
    DEPENDS ${MOD_LIBRARY_NAME}
)
