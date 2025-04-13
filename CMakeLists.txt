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
)

add_compile_options(${COMPILE_OPTIONS})
add_link_options(${COMPILE_OPTIONS})

include(cmake/stm32f103.cmake)

add_subdirectory(system)
add_subdirectory(uart)
add_subdirectory(utilities)
add_subdirectory(comm)
add_subdirectory(flash)
add_subdirectory(timer)
add_subdirectory(update)
add_subdirectory(boot)
add_subdirectory(third-party)

# Bootloader executable
set(BL_EXECUTABLE bootloader)
add_executable(${BL_EXECUTABLE} bootloader/main.c)

target_link_options(${BL_EXECUTABLE}
    PRIVATE
        -T ${CMAKE_SOURCE_DIR}/bootloader/linkerscript.ld
        -Wl,-Map=${BL_EXECUTABLE}.map
)

target_link_libraries(${BL_EXECUTABLE}
    PRIVATE
        stm32f103
        system
        uart
        comm
        update
        boot
)

# Generate executable as bin file
add_custom_command(TARGET ${BL_EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${BL_EXECUTABLE}> ${BL_EXECUTABLE}.bin
)


# Firmware executable
set(FW_EXECUTABLE firmware)
add_executable(${FW_EXECUTABLE} firmware/main.c)

target_sources(${FW_EXECUTABLE}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/firmware/header.c
)

target_link_options(${FW_EXECUTABLE}
    PRIVATE
        -T ${CMAKE_SOURCE_DIR}/firmware/linkerscript.ld
        -Wl,-Map=${FW_EXECUTABLE}.map
)

target_link_libraries(${FW_EXECUTABLE}
    PRIVATE
        stm32f103
        system
        boot
)

# Generate executable as bin file
add_custom_command(TARGET ${FW_EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${FW_EXECUTABLE}> ${FW_EXECUTABLE}.bin
)
