add_library(system INTERFACE)

target_sources(system
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/system.c
)

target_include_directories(system
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
