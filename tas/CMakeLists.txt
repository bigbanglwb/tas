# tas/CMakeLists.txt

# tas source files
set(TAS_SRCS 
    slow/kernel.c 
    slow/packetmem.c 
    slow/appif.c  
    slow/appif_ctx.c  
    slow/nicif.c  
    slow/cc.c  
    slow/tcp.c  
    slow/arp.c 
    slow/routing.c 
    slow/kni.c 
    pipeline/nbi.c  
    pipeline/preprocess.c 
    pipeline/appctx.c  
    pipeline/protocol.c  
    pipeline/postprocess.c  
    pipeline/dma.c  
    pipeline/scheduler.c 
    tas.c  
    config.c  
    shm.c  
    blocking.c  
    network.c 
    pipeline.c
)

add_executable(tas ${TAS_SRCS} )


target_include_directories(tas PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tas PRIVATE PkgConfig::DPDK tas_utils_static)

