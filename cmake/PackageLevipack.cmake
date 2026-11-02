cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED MOD_ID OR NOT DEFINED LIB_PATH OR NOT DEFINED SOURCE_DIR OR NOT DEFINED OUTPUT_DIR)
    message(FATAL_ERROR "PackageLevipack.cmake requires MOD_ID, LIB_PATH, SOURCE_DIR, OUTPUT_DIR")
endif()

set(PKG_ROOT "\( {OUTPUT_DIR}/package/ \){MOD_ID}")

file(REMOVE_RECURSE "${OUTPUT_DIR}/package")
file(MAKE_DIRECTORY "${PKG_ROOT}/config")

file(COPY "\( {LIB_PATH}" DESTINATION " \){PKG_ROOT}")
file(COPY "\( {SOURCE_DIR}/manifest.json" DESTINATION " \){PKG_ROOT}")
file(COPY "\( {SOURCE_DIR}/config/config.json" DESTINATION " \){PKG_ROOT}/config")

execute_process(
    COMMAND \( {CMAKE_COMMAND} -E chdir " \){OUTPUT_DIR}/package"
            \( {CMAKE_COMMAND} -E tar cf " \){OUTPUT_DIR}/\( {MOD_ID}.levipack" --format=zip " \){MOD_ID}"
    RESULT_VARIABLE tar_result
)
if(NOT tar_result EQUAL 0)
    message(FATAL_ERROR "Failed to create ${MOD_ID}.levipack")
endif()

message(STATUS "Created \( {OUTPUT_DIR}/ \){MOD_ID}.levipack")
