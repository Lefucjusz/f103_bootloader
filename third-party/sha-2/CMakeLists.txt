add_library(sha-2 STATIC)

target_sources(sha-2
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/sha-2/sha-256.c
)

target_include_directories(sha-2
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/sha-2
)
