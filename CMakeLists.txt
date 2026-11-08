cmake_minimum_required(VERSION 3.16)
project(stm32_reaction_game CXX C ASM)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Cross-compilation tools (set by toolchain file)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE arm-none-eabi-size)

# Compiler flags
set(CMAKE_C_FLAGS "-mcpu=cortex-m4 -mthumb -O0 -g -Wall -Wextra -DSTM32F446xx")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-m4 -mthumb -O0 -g -Wall -Wextra -DSTM32F446xx -fno-rtti -fno-exceptions")
set(CMAKE_ASM_FLAGS "-mcpu=cortex-m4 -mthumb")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -Wl,-Map=${CMAKE_BINARY_DIR}/output.map")

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Source files
set(SOURCES
    main.cpp
    system_stm32f4xx.c
    startup_stm32f446xx.s
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Vendored CMSIS headers (see third_party/README.md)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/cmsis-core/Include
    ${CMAKE_SOURCE_DIR}/third_party/cmsis-device-f4/Include
)

# Set startup file as assembly
set_source_files_properties(startup_stm32f446xx.s PROPERTIES LANGUAGE ASM)

# Set system file as C
set_source_files_properties(system_stm32f4xx.c PROPERTIES LANGUAGE C)

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX ".elf"
    LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_SOURCE_DIR}/stm32f446.ld"
)

# Create binary file from ELF
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "Creating binary file"
)
