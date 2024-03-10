# tests/CMakeLists.txt

# Source files for simple test programs
set(TESTS_NONE
    usocket_epoll_eof
    usocket_shutdown
)

# Source files for test programs linking against libtas
set(TESTS_LIBTAS
    lowlevel
    lowlevel_echo
    bench_ll_echo
)

# Source files for test programs linking against libtas_sockets
set(TESTS_SOCKETS
    usocket_accept
    usocket_connect
    usocket_accrx
    usocket_conntx
    usocket_conntx_large
    usocket_move
    usocket_bench
)

# Source files for automated unittests
set(TESTS_AUTO
    libtas/tas_ll
    libtas/tas_sockets
    tas_unit/fastpath
)

# Combine all test source files
set(TESTS_ALL ${TESTS_NONE} ${TESTS_LIBTAS} ${TESTS_SOCKETS} )

# Build executable for each test
foreach(TEST ${TESTS_ALL})
    add_executable(${TEST} ${TEST}.c)
    if (${TEST} IN_LIST TESTS_LIBTAS)
        target_include_directories(${TEST} PUBLIC ${CMAKE_SOURCE_DIR}/lib/tas/include)
        target_link_libraries(${TEST} PRIVATE PkgConfig::DPDK tas_lib tas_utils)  # Assuming you have a library target named tas_lib
    elseif (${TEST} IN_LIST TESTS_SOCKETS)
        target_include_directories(${TEST} PUBLIC ${CMAKE_SOURCE_DIR}/lib/sockets/include)
        target_link_libraries(${TEST} PRIVATE PkgConfig::DPDK tas_sockets tas_utils)  # Assuming you have a library target named tas_lib
    endif()
endforeach()

add_executable(tas_ll ${LIB_UTILS_SRCS} libtas/tas_ll.c libtas/harness.c testutils.c)
target_include_directories(tas_ll PUBLIC ${CMAKE_SOURCE_DIR}/lib/tas/include)
target_link_libraries(tas_ll PRIVATE PkgConfig::DPDK tas_lib tas_utils  )  # Assuming you have a library target named tas_lib

add_executable(test_tas_sockets ${LIB_UTILS_SRCS} libtas/tas_sockets.c libtas/harness.c testutils.c)
target_include_directories(test_tas_sockets PUBLIC ${CMAKE_SOURCE_DIR}/lib/tas/include)
target_link_libraries(test_tas_sockets PRIVATE  PkgConfig::DPDK tas_sockets tas_utils)  # Assuming you have
set_target_properties(test_tas_sockets PROPERTIES OUTPUT_NAME tas_sockets)

# Add a custom target to build all tests
add_custom_target(tests ALL DEPENDS ${TESTS_ALL})
