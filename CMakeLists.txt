cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_TOOLCHAIN_FILE cmake/arm-none-eabi-gcc.cmake)

project(f103_bootloader C ASM)

include(cmake/stm32f103.cmake)

add_executable(${CMAKE_PROJECT_NAME} main.c)

target_link_options(${CMAKE_PROJECT_NAME} 
    PRIVATE 
        -T ${CMAKE_SOURCE_DIR}/linkerscript.ld
)

add_subdirectory(system)
add_subdirectory(uart)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        stm32f103
        system
        uart
)
