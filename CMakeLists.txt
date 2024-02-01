# init config
cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(TAS)

add_compile_options(-std=gnu99 -O3 -g3 -Wall -march=native -fno-omit-frame-pointer)
add_link_options(-pthread -lm -lpthread -lrt -ldl)



# find dpdk
find_package(PkgConfig REQUIRED)
pkg_check_modules(DPDK REQUIRED IMPORTED_TARGET libdpdk)

# set(LIB_UTILS_SRCS 
#     ${CMAKE_SOURCE_DIR}/lib/utils/reorder.c 
#     ${CMAKE_SOURCE_DIR}/lib/utils/rng.c 
#     ${CMAKE_SOURCE_DIR}/lib/utils/timeout.c 
#     ${CMAKE_SOURCE_DIR}/lib/utils/utils.c )

# sub dir 
add_subdirectory(lib)
add_subdirectory(tas)
add_subdirectory(tests)



