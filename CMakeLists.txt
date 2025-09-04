cmake_minimum_required(VERSION 3.26)
project(bullet_hell_server LANGUAGES C CXX)

##### Project settings ###############################################
set(TARGET_NAME bullet_hell_server)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# Source directories
set(SRC_DIR src)

# Source files
set(SRC_FILES
    ${SRC_DIR}/game_server/game_server.cpp
    ${SRC_DIR}/game_server/game_server_utils.cpp
    ${SRC_DIR}/game_server/handle_client.cpp
    ${SRC_DIR}/game_logger/game_logger.cpp
)

##### External dependencies ##########################################
include(${CMAKE_SOURCE_DIR}/external/FetchGoogleTest.cmake)
include(${CMAKE_SOURCE_DIR}/external/FetchSol2.cmake)
include(${CMAKE_SOURCE_DIR}/external/FetchShared.cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external")
find_package(LuaJIT REQUIRED)

##### Main executable ################################################
add_executable(${TARGET_NAME}
    ${SRC_DIR}/main.cpp
    ${SRC_FILES}
)

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE src)

# Link libraries
target_link_libraries(${TARGET_NAME} PRIVATE
    luajit
    sol2::sol2
    shared_lib
)

# Define project root for compilation
target_compile_definitions(${TARGET_NAME} PRIVATE
    PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}"
)

##### Library for testing ############################################
# Create a static library from source files for testing
add_library(bullet_hell_lib STATIC ${SRC_FILES})

# Include directories for library
target_include_directories(bullet_hell_lib PUBLIC src)

# Link dependencies for library
target_link_libraries(bullet_hell_lib PRIVATE
    luajit
    sol2::sol2
    shared_lib
)

##### Google Test configuration ######################################
# Collect all test sources
file(GLOB TEST_SOURCES test/*.cpp)

# Create test executable
add_executable(all_tests ${TEST_SOURCES})

# Link test executable with library and gtest
target_link_libraries(all_tests PRIVATE bullet_hell_lib gtest_main)

# Enable automatic test discovery
include(GoogleTest)
gtest_discover_tests(all_tests)
