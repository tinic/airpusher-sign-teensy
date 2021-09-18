cmake_minimum_required(VERSION 3.10)

# IMX1062

project(airpusher-sign-teensy C CXX ASM)

set(CC_FLAGS
    -mcpu=cortex-m7 
    -mfpu=fpv5-sp-d16
    -mfloat-abi=hard
    -flto
    -mthumb
    -Wnull-dereference
    -fstrict-aliasing
    -fdata-sections
    -ffunction-sections
    -fno-common
    -fallow-store-data-races
    -Wall
    -Wno-format
    -ffast-math
    -fstack-usage)

set(CXX_FLAGS 
    -Wall 
    -Wextra
    -Wdouble-promotion
    -Wshadow 
    -Wfloat-conversion 
    -Wno-volatile
    -Wformat=2 
    -std=c++20 
    -fno-rtti 
    -fno-exceptions
    -fno-use-cxa-atexit)

set(LD_FLAGS
    -T${CMAKE_SOURCE_DIR}/MCUXpressoExtra/airpusher-sign.ld
    -flto
    -mcpu=cortex-m7
    -mfpu=fpv5-d16
    -mfloat-abi=hard
    -mthumb
    -ffast-math
    -specs=nano.specs
    -u _printf_float
    -Wl,-print-memory-usage
    -Wl,-Map=${PROJECT_NAME}.map,--cref
    -Wl,--gc-sections
    -Wl,--sort-section=alignment)

set(DEBUG_FLAGS
    -Og)

set(RELEASE_FLAGS   
    -Os)

set(DEFINITIONS
    CPU_MIMXRT1062DVL6A
    CPU_MIMXRT1062DVL6A_cm7
    SDK_OS_BAREMETAL
    SERIAL_PORT_TYPE_UART=1
    SDK_DEBUGCONSOLE=1
    SDK_DEBUGCONSOLE_UART 
    XIP_EXTERNAL_FLASH=1
    XIP_BOOT_HEADER_ENABLE=1
    __MCUXPRESSO 
    __USE_CMSIS
    __NEWLIB__
    __IMXRT1062__)

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)
set(DUMP_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.S)

include_directories(.)
include_directories()
include_directories(MCUXpresso/board)
include_directories(MCUXpresso/source)
include_directories(MCUXpresso/source/generated)
include_directories(MCUXpresso/CMSIS_driver)
include_directories(MCUXpresso/drivers)
include_directories(MCUXpresso/CMSIS/DSP/Include)
include_directories(MCUXpresso/component/serial_manager)
include_directories(MCUXpresso/component/uart)
include_directories(MCUXpresso/component/osa)
include_directories(MCUXpresso/component/lists)
include_directories(MCUXpresso/utilities)
include_directories(MCUXpresso/mdio)
include_directories(MCUXpresso/phy)
include_directories(MCUXpresso/CMSIS)
include_directories(MCUXpresso/device)
include_directories(MCUXpresso/usb/device/include)
include_directories(MCUXpresso/usb/device/source)
include_directories(MCUXpresso/usb/device/source/ehci)
include_directories(MCUXpresso/usb/device/class/)
include_directories(MCUXpresso/usb/device/class/audio)
include_directories(MCUXpresso/usb/device/class/ccid)
include_directories(MCUXpresso/usb/device/class/cdc)
include_directories(MCUXpresso/usb/device/class/cdc_rndis)
include_directories(MCUXpresso/usb/device/class/dfu)
include_directories(MCUXpresso/usb/device/class/hid)
include_directories(MCUXpresso/usb/device/class/msc)
include_directories(MCUXpresso/usb/device/class/phdc)
include_directories(MCUXpresso/usb/device/class/printer)
include_directories(MCUXpresso/usb/device/class/video)
include_directories(MCUXpresso/usb/include)
include_directories(MCUXpresso/usb/phy)

file(GLOB_RECURSE MCUXPRESSO_SRC_0 ${PROJECT_SOURCE_DIR}/MCUXpresso/board/*.c)
file(GLOB_RECURSE MCUXPRESSO_SRC_1 ${PROJECT_SOURCE_DIR}/MCUXpresso/component/lists/*.c)
file(GLOB_RECURSE MCUXPRESSO_SRC_2 ${PROJECT_SOURCE_DIR}/MCUXpresso/component/serial_manager/*.c)
file(GLOB_RECURSE MCUXPRESSO_SRC_3 ${PROJECT_SOURCE_DIR}/MCUXpresso/component/uart/*.c)
file(GLOB_RECURSE MCUXPRESSO_SRC_4 ${PROJECT_SOURCE_DIR}/MCUXpresso/device/*.c)
file(GLOB_RECURSE MCUXPRESSO_SRC_5 ${PROJECT_SOURCE_DIR}/MCUXpresso/drivers/*.c)
file(GLOB_RECURSE MCUXPRESSO_SRC_6 ${PROJECT_SOURCE_DIR}/MCUXpresso/source/*.c)
file(GLOB_RECURSE MCUXPRESSO_SRC_7 ${PROJECT_SOURCE_DIR}/MCUXpresso/startup/*.c)
file(GLOB_RECURSE MCUXPRESSO_SRC_8 ${PROJECT_SOURCE_DIR}/MCUXpresso/utilities/*.c)
file(GLOB_RECURSE MCUXPRESSO_SRC_9 ${PROJECT_SOURCE_DIR}/MCUXpresso/source/*.cpp)
file(GLOB_RECURSE MCUXPRESSO_SRC_10 ${PROJECT_SOURCE_DIR}/MCUXpresso/startup/*.cpp)
file(GLOB_RECURSE MCUXPRESSO_SRC_11 ${PROJECT_SOURCE_DIR}/MCUXpresso/xip/*.c)
file(GLOB_RECURSE MCUXPRESSO_SRC_12 ${PROJECT_SOURCE_DIR}/MCUXpresso/usb/*.c)

add_executable(${PROJECT_NAME}.elf 
    MCUXpressoExtra/syscalls.c
    MCUXpressoExtra/sysmem.c
    ${MCUXPRESSO_SRC_0}
    ${MCUXPRESSO_SRC_1}
    ${MCUXPRESSO_SRC_2}
    ${MCUXPRESSO_SRC_3}
    ${MCUXPRESSO_SRC_4}
    ${MCUXPRESSO_SRC_5}
    ${MCUXPRESSO_SRC_6}
    ${MCUXPRESSO_SRC_7}
    ${MCUXPRESSO_SRC_8}
    ${MCUXPRESSO_SRC_9}
    ${MCUXPRESSO_SRC_10}
    ${MCUXPRESSO_SRC_11}
    ${MCUXPRESSO_SRC_12})

# Generate version.h
find_package(Git)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_SHORT_SHA OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-list HEAD --count WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_REV_COUNT OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} show -s --format=%ad HEAD --date=iso-strict WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_COMMIT_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    set(GIT_SHORT_SHA "unknown")
    set(GIT_REV_COUNT "unknown")
    set(GIT_COMMIT_DATE "unknown")
endif()

include_directories(${CMAKE_BINARY_DIR})
configure_file("${PROJECT_SOURCE_DIR}/version.h.in" "${CMAKE_BINARY_DIR}/version.h" @ONLY)

include(./cmake/utils.cmake)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    include(./cmake/clang_utils.cmake)
    clang_utils_get_arm_gcc_version(${ARM_GCC_C_COMPILER})
    clang_utils_get_arm_gcc_sysroot(${ARM_GCC_C_COMPILER} ${CC_FLAGS})
    clang_utils_get_arm_gcc_multilib_dir(${ARM_GCC_C_COMPILER} ${CC_FLAGS})
    clang_utils_get_arm_gcc_lib_dir(${ARM_GCC_C_COMPILER} ${CC_FLAGS})
    clang_utils_get_arm_gcc_crt(${ARM_GCC_SYSROOT} ${ARM_GCC_MULTIDIR} ${ARM_GCC_LIBGCC_DIR})
    string(APPEND CMAKE_EXE_LINKER_FLAGS" -nostdlib -L${ARM_GCC_SYSROOT}/lib/${ARM_GCC_MULTIDIR} -L${ARM_GCC_LIBGCC_DIR}")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -lgcc -lnosys -lc")
	include_directories(${ARM_GCC_SYSROOT}/include/c++/${ARM_GCC_VERSION})
	include_directories(${ARM_GCC_SYSROOT}/include/c++/${ARM_GCC_VERSION}/arm-none-eabi/${ARM_GCC_MULTIDIR})
	if(ARM_GCC_VERSION LESS "10.2")
		message(FATAL_ERROR "GCC must be version 10.2 or higher.")
	endif(ARM_GCC_VERSION LESS "10.2")
else()
	if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		message(FATAL_ERROR "Compiler must be GCC.")
	endif(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_CXX_COMPILER_VERSION LESS "10.2")
		message(FATAL_ERROR "GCC must be version 10.2 or higher.")
	endif(CMAKE_CXX_COMPILER_VERSION LESS "10.2")
endif()

target_compile_definitions(${PROJECT_NAME}.elf PRIVATE ${DEFINITIONS})
target_compile_options(${PROJECT_NAME}.elf PRIVATE ${CC_FLAGS})
target_link_options(${PROJECT_NAME}.elf PRIVATE ${LD_FLAGS})
target_compile_options(${PROJECT_NAME}.elf PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CXX_FLAGS}>")
target_compile_options(${PROJECT_NAME}.elf PRIVATE "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>")
target_compile_options(${PROJECT_NAME}.elf PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -D $<TARGET_FILE:${PROJECT_NAME}.elf> > ${DUMP_FILE}
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
    COMMAND ${CMAKE_OBJCOPY} -O ihex -R .eeprom -R .fuse -R .lock -R .signature $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
    COMMENT "Building ${HEX_FILE} \nBuilding ${BIN_FILE}")
    
set(BASE_ADDRESS 0x08000000)
set(PROGRAM_CMD "./openocd -f stlink.cfg -f stm32h7x.cfg -c \"program ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin ${BASE_ADDRESS} verify reset exit\"")
install(CODE "execute_process(COMMAND ${PROGRAM_CMD} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/openocd)")
