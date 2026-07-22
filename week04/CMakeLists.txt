cmake_minimum_required(VERSION 3.20)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/arm-none-eabi-gcc.cmake)

project(HW04_GPIO C ASM)

set(CMAKE_C_STANDARD 11)
set(PROJECT_NAME HW04_GPIO)

add_executable(${PROJECT_NAME}
    startup_stm32f103xb.s
)

set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".elf")

target_sources(${PROJECT_NAME} PRIVATE
    Src/main.c
    Src/stm32f1xx_hal.c
    Src/stm32f1xx_hal_gpio.c
    Src/stm32f1xx_hal_rcc.c
    Src/stm32f1xx_hal_cortex.c
    Src/system_stm32f1xx.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    Inc
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    STM32F103xB
    USE_HAL_DRIVER
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -mfloat-abi=soft
    -Wall
    -O0
    -g3
    -ffunction-sections
    -fdata-sections
)

target_link_options(${PROJECT_NAME} PRIVATE
    -T${CMAKE_SOURCE_DIR}/STM32F103C8Tx_FLASH.ld
    -mcpu=cortex-m3
    -mthumb
    -mfloat-abi=soft
    --specs=nano.specs
    -nostartfiles
    -Wl,--gc-sections
    -Wl,-Map=${PROJECT_NAME}.map
)

# Tự động tạo file .bin và .hex sau khi build xong (để nạp vào chip)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
)