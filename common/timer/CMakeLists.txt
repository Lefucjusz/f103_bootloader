add_library(timer INTERFACE)

target_sources(timer
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/timer.c
)

target_include_directories(timer
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(timer
    INTERFACE
        system
)
