add_library(comm INTERFACE)

target_sources(comm
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/comm.c
)

target_include_directories(comm
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(comm
    INTERFACE
        utils
        ring_buffer
)
