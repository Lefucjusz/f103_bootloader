cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_TOOLCHAIN_FILE cmake/arm-none-eabi-gcc.cmake)

project(f103_bootloader C ASM)

set(COMPILE_OPTIONS
    --static
    -nostartfiles
    -fno-common
    -mcpu=cortex-m3
    -mthumb
    -mfloat-abi=soft
    -Wl,--print-memory-usage
    -Wl,-Map=${CMAKE_PROJECT_NAME}.map
)

add_compile_options(${COMPILE_OPTIONS})
add_link_options(${COMPILE_OPTIONS})

include(cmake/stm32f103.cmake)

set(EXECUTABLE ${CMAKE_PROJECT_NAME})
add_executable(${EXECUTABLE} main.c)

target_link_options(${EXECUTABLE}
    PRIVATE 
        -T ${CMAKE_SOURCE_DIR}/linkerscript.ld
)

add_subdirectory(system)
add_subdirectory(uart)
add_subdirectory(utilities)
add_subdirectory(comm)
add_subdirectory(flash)
add_subdirectory(timer)
add_subdirectory(update)
add_subdirectory(boot)
add_subdirectory(third-party)

target_link_libraries(${EXECUTABLE}
    PRIVATE
        stm32f103
        system
        uart
        comm
        update
        boot
)

# Generate executable as bin file
add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${EXECUTABLE}> ${EXECUTABLE}.bin
)
