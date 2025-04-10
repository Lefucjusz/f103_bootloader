add_library(system INTERFACE)

target_sources(system
    INTERFACE
        system.c
)

target_include_directories(system
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
