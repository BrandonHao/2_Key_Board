cmake_minimum_required(VERSION 3.18.0)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CWD ${CMAKE_CURRENT_SOURCE_DIR})
set(TINYUSB ${CWD}/tinyusb)
set(TINYUSB_SRC ${TINYUSB}/src)
set(TINYUSB_HW ${TINYUSB}/hw)
set(CMSIS ${CWD}/cmsis)
set(CORE ${CWD}/Core)
set(CMAKE_TOOLCHAIN_FILE ${CWD}/arm-none-eabi-gcc.cmake)

project(Keyboard C)

file(GLOB CMSIS_INC
    "${CMSIS}/core_cm0plus.h"
    "${CMSIS}/core_cm0Func.h"
    "${CMSIS}/core_cm0Instr.h"
    "${CMSIS}/sam.h"
    "${CMSIS}/samd21e17a.h")

file(GLOB TINYUSB_SOURCE
    "${TINYUSB_SRC}/tusb.c"
    "${TINYUSB_SRC}/portable/microchip/samd/dcd_samd.c"
    "${TINYUSB_SRC}/class/hid/hid_device.c"
    "${TINYUSB_SRC}/class/hid/hid.c"
    "${TINYUSB_SRC}/device/usbd_control.c"
    "${TINYUSB_SRC}/device/usbd.c"
    "${TINYUSB_SRC}/common/tusb_fifo.c"
    "${TINYUSB_HW}/bsp/samd21/family.c"
	"${TINYUSB_HW}/mcu/microchip/samd21/hpl/gclk/hpl_gclk.c"
	"${TINYUSB_HW}/mcu/microchip/samd21/hpl/pm/hpl_pm.c"
	"${TINYUSB_HW}/mcu/microchip/samd21/hpl/sysctrl/hpl_sysctrl.c"
	"${TINYUSB_HW}/mcu/microchip/samd21/hal/src/hal_atomic.c")

file(GLOB TINYUSB_INC
    "${TINYUSB_SRC}/tusb.h"
    "${TINYUSB_SRC}/tusb_option.h"
    "${TINYUSB_SRC}/class/hid/hid_device.h"
    "${TINYUSB_SRC}/class/hid/hid.h"
    "${TINYUSB_SRC}/common/tusb_common.h"
    "${TINYUSB_SRC}/common/tusb_compiler.h"
    "${TINYUSB_SRC}/common/tusb_error.h"
    "${TINYUSB_SRC}/common/tusb_fifo.h"
    "${TINYUSB_SRC}/common/tusb_timeout.h"
    "${TINYUSB_SRC}/common/tusb_types.h"
    "${TINYUSB_SRC}/common/tusb_verify.h"
    "${TINYUSB_SRC}/device/dcd.h"
    "${TINYUSB_SRC}/device/dcd_attr.h"
    "${TINYUSB_SRC}/device/usbd_pvt.h"
    "${TINYUSB_SRC}/device/usbd.h"
    "${TINYUSB}/hw/mcu/microchip/samd21/hal/include/hal_gpio.h")

file(GLOB PROJ_SOURCE
    "${CORE}/src/main.c"
    "${CORE}/src/usb_descriptors.c"
    "${CORE}/src/startup_samd21.c"
    "${CORE}/src/timer.c"
    "${CORE}/src/ext_interrupt.c")

file(GLOB PROJ_INC
    "${CORE}/inc/tusb_config.h"
    "${CORE}/inc/timer.h"
    "${CORE}/inc/ext_interrupt.h")

set(LINKER_PATH "${CWD}/linker/samd21e17d.ld")

enable_language(C ASM)
set(CMAKE_C_STANDARD 11)

set(EXECUTABLE ${PROJECT_NAME}.elf)
add_executable(${EXECUTABLE} ${PROJ_SOURCE} ${PROJ_INC} ${CMSIS_INC} ${TINYUSB_SOURCE} ${TINYUSB_INC})

target_include_directories(${EXECUTABLE} PRIVATE
    cmsis
    cmsis/component
    cmsis/instance
    cmsis/pio
    tinyusb/src
    tinyusb/src/class/hid
    tinyusb/src/common
    tinyusb/src/device
    tinyusb/hw
    tinyusb/hw/bsp
    tinyusb/hw/mcu/microchip/samd21
	tinyusb/hw/mcu/microchip/samd21/config
	tinyusb/hw/mcu/microchip/samd21/include
	tinyusb/hw/mcu/microchip/samd21/hal/include
	tinyusb/hw/mcu/microchip/samd21/hal/utils/include
	tinyusb/hw/mcu/microchip/samd21/hpl/pm
	tinyusb/hw/mcu/microchip/samd21/hpl/port
	tinyusb/hw/mcu/microchip/samd21/hri
	tinyusb/hw/mcu/microchip/samd21/CMSIS/Include
    core/inc
    core/src)

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
    -g
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