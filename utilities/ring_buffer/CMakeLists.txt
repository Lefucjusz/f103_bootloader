add_library(ring_buffer INTERFACE)

target_sources(ring_buffer
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/ring_buffer.c
)

target_include_directories(ring_buffer
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
