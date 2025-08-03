# Optional override
set(LUAJIT_ROOT_DIR "" CACHE PATH "Root directory of LuaJIT installation")

if (WIN32)
    if (MSVC)
        set(LUAJIT_LIB_PATH "${CMAKE_SOURCE_DIR}/external/LuaJIT/src/luajit.lib")
    else()
        set(LUAJIT_LIB_PATH "${CMAKE_SOURCE_DIR}/external/LuaJIT/src/libluajit.a")
    endif()

    add_library(luajit STATIC IMPORTED GLOBAL)
    set_target_properties(luajit PROPERTIES
        IMPORTED_LOCATION "${LUAJIT_LIB_PATH}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/external/LuaJIT/src"
    )
else()
    find_path(LUAJIT_INCLUDE_DIR
        NAMES luajit.h
        PATH_SUFFIXES luajit-2.1
        HINTS ${LUAJIT_ROOT_DIR}/include
        PATHS /usr/include /usr/local/include
    )

    find_library(LUAJIT_LIBRARY
        NAMES luajit luajit-5.1
        HINTS ${LUAJIT_ROOT_DIR}/lib
        PATHS /usr/lib /usr/local/lib
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(LuaJIT DEFAULT_MSG LUAJIT_INCLUDE_DIR LUAJIT_LIBRARY)

    if (NOT LUAJIT_FOUND)
        message(FATAL_ERROR "LuaJIT not found. Please install it or set LUAJIT_ROOT_DIR.")
    endif()

    add_library(luajit INTERFACE)
    target_include_directories(luajit INTERFACE ${LUAJIT_INCLUDE_DIR})
    target_link_libraries(luajit INTERFACE ${LUAJIT_LIBRARY})
endif()
