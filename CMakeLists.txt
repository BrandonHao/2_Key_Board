cmake_minimum_required(VERSION 3.18.0)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CWD ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_TOOLCHAIN_FILE ${CWD}/arm-none-eabi-gcc.cmake)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(Keyboard C)

file(GLOB CMSIS_SOURCE
    "${CWD}/cmsis/core_cm0plus.h"
    "${CWD}/cmsis/core_cm0Func.h"
    "${CWD}/cmsis/core_cm0Instr.h"
    "${CWD}/cmsis/samd21.h"
    "${CWD}/cmsis/samd21e17a.h"
    "${CWD}/core/inc/hal_gpio.h")

file(GLOB PROJ_SOURCE
    "${CWD}/Core/Src/main.c"
    "${CWD}/Core/Src/startup_samd21.c")

set(LINKER_PATH "${CWD}/linker/samd21e17d.ld")

enable_language(C ASM)
set(CMAKE_C_STANDARD 11)

set(EXECUTABLE ${PROJECT_NAME}.elf)
add_executable(${EXECUTABLE} ${PROJ_SOURCE} ${CMSIS_SOURCE})

target_include_directories(${EXECUTABLE} PRIVATE
    cmsis
    cmsis/component
    cmsis/instance
    cmsis/pio
    core/inc)

target_compile_definitions(${EXECUTABLE} PRIVATE
    -DDEBUG
    -DF_CPU=8000000 
    -DDONT_USE_CMSIS_INIT 
    -D__SAMD21E17A__)

target_compile_options(${EXECUTABLE} PRIVATE
    -W 
    -Wall 
    --std=gnu11 
    -O0
    -fno-diagnostics-show-caret
    -fdata-sections 
    -ffunction-sections
    -funsigned-char 
    -funsigned-bitfields
    -mcpu=cortex-m0plus 
    -mthumb)

target_link_options(${EXECUTABLE} PRIVATE
    -mcpu=cortex-m0plus 
    -mthumb
    -Wl,--gc-sections
    -Wl,--script=${LINKER_PATH})

set(OUTPUT_FILE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME})

add_custom_command(TARGET ${EXECUTABLE}
    POST_BUILD
    COMMAND arm-none-eabi-size --format=sysv ${OUTPUT_FILE}.elf)

# Create hex file
add_custom_command(TARGET ${EXECUTABLE}
    POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex ${OUTPUT_FILE}.elf ${OUTPUT_FILE}.hex
    COMMAND arm-none-eabi-objcopy -O binary ${OUTPUT_FILE}.elf ${OUTPUT_FILE}.bin)