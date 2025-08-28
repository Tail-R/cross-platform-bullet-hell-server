cmake_minimum_required(VERSION 3.26)
project(bullet_hell_server LANGUAGES C CXX)

# Target Name
set(TARGET_NAME bullet_hell_server)

# c++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# c version
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
    # Entry point
    ${SRC_DIR}/main.cpp

    ${SRC_DIR}/game_server/game_server.cpp
    ${SRC_DIR}/game_server/game_server_utils.cpp
    ${SRC_DIR}/game_server/handle_client.cpp
)

# Include scripts
include(${CMAKE_SOURCE_DIR}/external/FetchSol2.cmake)
include(${CMAKE_SOURCE_DIR}/external/FetchShared.cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external")
find_package(LuaJIT REQUIRED)

# Executable
add_executable(${TARGET_NAME} ${SRC_FILES})

# Include paths
target_include_directories(${TARGET_NAME} PRIVATE
    src
)

# Link libraries
target_link_libraries(${TARGET_NAME} PRIVATE
    luajit
    sol2::sol2
    shared_lib
)

# Link OS-specific libraries
# Winsock2
if(WIN32)
    if (MINGW)
        target_link_libraries(${TARGET_NAME} PRIVATE mingw32)
    endif()

    target_link_libraries(${TARGET_NAME} PRIVATE ws2_32)
endif()

target_compile_definitions(${TARGET_NAME} PRIVATE
    PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}"
)