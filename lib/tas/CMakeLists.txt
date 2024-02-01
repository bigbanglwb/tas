set(LIB_TAS_SRCS 
    init.c 
    kernel.c 
    conn.c 
    connect.c )

add_library(tas_lib SHARED  ${LIB_TAS_SRCS})
target_include_directories(tas_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tas_lib PRIVATE tas_utils)
set_target_properties(tas_lib PROPERTIES OUTPUT_NAME tas)