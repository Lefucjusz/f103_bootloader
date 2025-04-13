add_library(micro-ecc STATIC)

target_sources(micro-ecc
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/micro-ecc/uECC.c
)

target_include_directories(micro-ecc
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/micro-ecc
)
