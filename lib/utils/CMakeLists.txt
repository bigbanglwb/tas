set(LIB_UTILS_SRCS
    reorder.c 
    rng.c 
    timeout.c 
    utils.c )


add_library (tas_utils SHARED  ${LIB_UTILS_SRCS})
target_include_directories(tas_utils PUBLIC ${CMAKE_SOURCE_DIR}/include)


add_library(tas_utils_static STATIC ${LIB_UTILS_SRCS})
target_include_directories(tas_utils_static PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(tas_utils_static PROPERTIES OUTPUT_NAME tas_utils)
