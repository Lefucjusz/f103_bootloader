add_library(uart INTERFACE)

target_sources(uart
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/uart.c
)

target_include_directories(uart
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(uart
    INTERFACE
        ring_buffer
)
