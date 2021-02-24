cmake_minimum_required(VERSION 3.1)

project(pwng-server)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    add_compile_options(-mtune=native -Wall -pedantic)
else()
    add_compile_options(-mtune=native -Wall -Wextra -pedantic)
endif()

set(BOX2D_LIBRARY_LOCAL "${PROJECT_SOURCE_DIR}/install/lib/libbox2d.a")
# set(LIBNOISE_LIBRARY_LOCAL
#         ${PROJECT_SOURCE_DIR}/install/lib/libnoise2d.so
# )
# set(LIB_NOISE_HEADERS
#         ${PROJECT_SOURCE_DIR}/3rdparty/libnoise2d/src/noise.h
# )

add_subdirectory(./src)
