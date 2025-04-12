add_library(boot INTERFACE)

target_sources(boot
    INTERFACE
        boot.c
)

target_include_directories(boot
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(update
    INTERFACE
        flash
)
