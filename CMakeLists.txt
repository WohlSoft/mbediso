cmake_minimum_required(VERSION 3.2...3.5)

include(TestBigEndian)
include(GNUInstallDirs)

if(POLICY CMP0069) # Allow CMAKE_INTERPROCEDURAL_OPTIMIZATION (lto) to be set
    cmake_policy(SET CMP0069 NEW)
endif()

project(mbediso VERSION 0.0 LANGUAGES C)

set(MBEDISO_SRC
    src/internal/directory.c
    src/internal/fs.c
    src/internal/io.c
    src/internal/read.c
    src/internal/string_diff.c
    src/internal/util.c
    src/internal/lz4_header.c
    src/public/file.c
    src/public/dir.c
    src/public/fs.c
)

if(NOT ANDROID
   AND NOT IOS
   AND NOT TVOS
   AND NOT WATCHOS
   AND NOT VISIONOS
   AND NOT PLATFORM_IOS
   AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS"
   AND NOT NINTENDO_WII
   AND NOT NINTENDO_WIIU
   AND NOT NINTENDO_DS
   AND NOT NINTENDO_3DS
   AND NOT VITA
   AND NOT PSP
   AND NOT PS2
   AND NOT EMSCRIPTEN)
    option(MBEDISO_BUILD_EXECUTABLES "Build executables included with the project" ON)
endif()

option(USE_EXTERNAL_LZ4 "Use externally provided LZ4" OFF)

set(MBEDISO_THREADS_DEFAULT "NONE")
set(MBEDISO_THREADS "${MBEDISO_THREADS_DEFAULT}" CACHE STRING "Threading library for mbediso [NONE, ...]")

if("${MBEDISO_THREADS}" STREQUAL "NONE")
    message("== mbediso will be built without mutex support. The resulting library is not thread safe.")
    list(APPEND MBEDISO_SRC src/internal/mutex/mutex_none.c)
elseif("${MBEDISO_THREADS}" STREQUAL "SDL2")
    message("== mbediso will be built with SDL2 mutex support.")
    list(APPEND MBEDISO_SRC src/internal/mutex/mutex_sdl2.c)
else()
    message("== mbediso will be built with application-provided mutex support.")
endif()

if(USE_EXTERNAL_LZ4)
    add_library(lz4_static INTERFACE)
    set(LZ4_INCLUDE_DIRS "/usr/include" CACHE STRING "Include path to the LZ4 library")
    set(LZ4_LIBRARIES "lz4_pack_static;lz4" CACHE STRING "LZ4 libraries to link")
    target_include_directories(lz4_static INTERFACE "${LZ4_INCLUDE_DIRS}")
    target_link_libraries(lz4_static INTERFACE ${LZ4_LIBRARIES})
else()
    add_subdirectory(lz4/build/cmake EXCLUDE_FROM_ALL)
endif()

add_library(mbediso STATIC ${MBEDISO_SRC})

test_big_endian(MBEDISO_BIG_ENDIAN)
if(MBEDISO_BIG_ENDIAN)
    target_compile_definitions(mbediso PRIVATE -DMBEDISO_BIG_ENDIAN=1)
else()
    target_compile_definitions(mbediso PRIVATE -DMBEDISO_BIG_ENDIAN=0)
endif()

target_link_libraries(mbediso PRIVATE lz4_static)

target_include_directories(mbediso PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(mbediso PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(TARGETS mbediso
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(FILES include/mbediso.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(DIRECTORY include/mbediso
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

if(NOT USE_EXTERNAL_LZ4)
    install(FILES
        lz4/lib/lz4.h
        lz4/lib/lz4hc.h
        lz4/lib/lz4file.h
        lz4/lib/lz4frame.h
        lz4/lib/lz4frame_static.h
    DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/mbediso/lz4"
    )
endif()

if(NOT MSVC)
    target_compile_options(mbediso PRIVATE -Wall -Wextra -pedantic)
endif()

if(MBEDISO_BUILD_EXECUTABLES)
    # example utility to allow user to read from an ISO
    add_executable(read examples/main.c)
    target_link_libraries(read PRIVATE mbediso)
endif()

# lz4 pack command line util
project(lz4_pack VERSION 0.0 LANGUAGES CXX)

add_library(lz4_pack_static util/lz4_pack/lz4_pack.cpp)
target_include_directories(lz4_pack_static PUBLIC util/lz4_pack/include)
set_target_properties(lz4_pack_static PROPERTIES PUBLIC_HEADER "util/lz4_pack/include/lz4_pack.h")
target_link_libraries(lz4_pack_static PUBLIC lz4_static)

if(MBEDISO_BUILD_EXECUTABLES)
    add_executable(lz4_pack_cli util/lz4_pack/main.cpp)
    target_link_libraries(lz4_pack_cli PRIVATE lz4_pack_static)

    install(TARGETS lz4_pack_cli
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
endif()

install(TARGETS lz4_pack_static
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
