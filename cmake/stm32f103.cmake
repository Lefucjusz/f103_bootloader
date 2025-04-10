set(OPENCM3_ROOT_DIR ${CMAKE_SOURCE_DIR}/third-party/libopencm3)

# Add target to build libopencm3 for the target that will be used
add_custom_target(libopencm3 make -j$(nproc) TARGETS=stm32/f1 WORKING_DIRECTORY ${OPENCM3_ROOT_DIR})

# Create a specific CPU target
add_library(stm32f103 STATIC IMPORTED)
set_property(TARGET stm32f103 PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${OPENCM3_ROOT_DIR}/include)
set_property(TARGET stm32f103 PROPERTY IMPORTED_LOCATION ${OPENCM3_ROOT_DIR}/lib/libopencm3_stm32f1.a)
add_dependencies(stm32f103 libopencm3)
target_link_directories(stm32f103 INTERFACE ${OPENCM3_ROOT_DIR}/lib)

target_compile_definitions(stm32f103 INTERFACE STM32F1)

set(COMPILE_OPTIONS
    --static
    -nostartfiles
    -fno-common
    -mcpu=cortex-m3
    -mfloat-abi=soft
    -Wl,--print-memory-usage
    -Wl,-Map=${CMAKE_PROJECT_NAME}.map
)

target_compile_options(stm32f103 INTERFACE ${COMPILE_OPTIONS})
target_link_options(stm32f103 INTERFACE ${COMPILE_OPTIONS})
