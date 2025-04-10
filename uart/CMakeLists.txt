add_library(uart INTERFACE)

target_sources(uart
    INTERFACE
        uart.c
)

target_include_directories(uart
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
