add_library(update INTERFACE)

target_sources(update
    INTERFACE
        update.c
)

target_include_directories(update
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(update
    INTERFACE
        uart
        comm
        timer
        utils
        flash
        system
        tiny-aes
)
