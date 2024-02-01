set(LIB_SOCKETS_SRCS 
control.c transfer.c context.c manage_fd.c epoll.c poll.c libc.c )

set(LIB_SINT_SRCS 
interpose.c)

add_library(tas_sockets SHARED  ${LIB_SOCKETS_SRCS})
target_include_directories(tas_sockets PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tas_sockets PRIVATE tas_lib tas_utils)


add_library(tas_interpose SHARED  ${LIB_SINT_SRCS})
target_include_directories(tas_interpose PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tas_interpose PRIVATE tas_lib tas_utils)
