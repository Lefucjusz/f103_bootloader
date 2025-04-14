add_library(flash INTERFACE)

target_sources(flash
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/flash.c
)

target_include_directories(flash
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
