set(TAS_TOOLS 
    scaletool 
    tracetool 
)

foreach(TOOL ${TAS_TOOLS})
    add_executable(${TOOL} ${TOOL}.c)
    target_include_directories(${TOOL} PUBLIC ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${TOOL} PRIVATE tas_lib)
endforeach()