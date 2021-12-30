cmake_minimum_required(VERSION 3.18.0)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CWD ${CMAKE_CURRENT_SOURCE_DIR})
set(TINYUSB ${CWD}/tinyusb)
set(CMSIS ${CWD}/cmsis)
set(CORE ${CWD}/Core)
set(HAL ${CWD}/hw/hal)
set(HPL ${CWD}/hw/hpl)
set(CMAKE_TOOLCHAIN_FILE ${CWD}/arm-none-eabi-gcc.cmake)

project(Keyboard C)

file(GLOB CMSIS_INC
    "${CMSIS}/core_cm0plus.h"
    "${CMSIS}/core_cm0Func.h"
    "${CMSIS}/core_cm0Instr.h"
    "${CMSIS}/sam.h"
    "${CMSIS}/samd21e17a.h")

file(GLOB HW_INC
    "${HAL}/include/hal_gpio.h"
    "${HPL}/pm/hpl_pm_base.h"
    "${HPL}/gclk/hpl_gclk_base.h")

file(GLOB HW_SRC
    "${HPL}/gclk/hpl_gclk.c"
    "${HPL}/pm/hpl_pm.c"
    "${HPL}/sysctrl/hpl_sysctrl.c")

file(GLOB TINYUSB_SOURCE
    "${TINYUSB}/tusb.c"
    "${TINYUSB}/class/hid_device.c"
    "${TINYUSB}/class/hid.c"
    "${TINYUSB}/common/tusb_fifo.c"
    "${TINYUSB}/device/usbd_control.c"
    "${TINYUSB}/device/usbd.c"
    "${TINYUSB}/portable/dcd_samd.c")

file(GLOB TINYUSB_INC
    "${TINYUSB}/tusb.h"
    "${TINYUSB}/tusb_option.h"
    "${TINYUSB}/class/hid_device.h"
    "${TINYUSB}/class/hid.h"
    "${TINYUSB}/common/tusb_common.h"
    "${TINYUSB}/common/tusb_compiler.h"
    "${TINYUSB}/common/tusb_error.h"
    "${TINYUSB}/common/tusb_fifo.h"
    "${TINYUSB}/common/tusb_timeout.h"
    "${TINYUSB}/common/tusb_types.h"
    "${TINYUSB}/common/tusb_verify.h"
    "${TINYUSB}/device/dcd.h"
    "${TINYUSB}/device/dcd_attr.h"
    "${TINYUSB}/device/usbd_pvt.h"
    "${TINYUSB}/device/usbd.h")

file(GLOB PROJ_SOURCE
    "${CORE}/src/startup_samd21.c"
    "${CORE}/src/main.c"
    "${CORE}/src/usb_descriptors.c"
    "${CORE}/src/timer.c")

file(GLOB PROJ_INC
    "${CORE}/inc/tusb_config.h"
    "${CORE}/inc/timer.h")

set(LINKER_PATH "${CWD}/linker/samd21e17d.ld")

enable_language(C ASM)
set(CMAKE_C_STANDARD 11)

set(EXECUTABLE ${PROJECT_NAME}.elf)
add_executable(${EXECUTABLE} ${PROJ_SOURCE} ${PROJ_INC} ${CMSIS_INC} ${HW_INC} ${HW_SRC} ${TINYUSB_SOURCE} ${TINYUSB_INC})

target_include_directories(${EXECUTABLE} PRIVATE
    cmsis
    cmsis/component
    cmsis/instance
    cmsis/pio
    tinyusb
    tinyusb/class
    tinyusb/common
    tinyusb/device
	hw/config
	hw/hal/include
	hw/hal/utils/include
	hw/hpl/gclk
	hw/hpl/pm
	hw/hpl/port
	hw/hri
    core/inc)

target_compile_definitions(${EXECUTABLE} PRIVATE
    -DDEBUG
    -DUSE_SIMPLE_ASSERT
    -DF_CPU=8000000 
    -DDONT_USE_CMSIS_INIT 
    -D__SAMD21E17A__
    -DCFG_TUSB_MCU=OPT_MCU_SAMD21)

target_compile_options(${EXECUTABLE} PRIVATE
    -W 
    -Wall 
    -Wextra
    -Werror
    --std=gnu11 
    -O3
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