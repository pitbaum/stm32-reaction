
# C++ Template for the Nucleo 144 (STM32L4466RE)

Using a minimal C++ project template for STM32L4466RE microcontroller with CMake build system and integrated debugging support by Tom Schammo.

Implementing a reaction timer game with LED, GPIO, UART and Interrupts.

## Features

- **C++17** support with proper ARM cross-compilation
- **CMake** build system with Makefile convenience wrapper
- **Integrated debugging** with OpenOCD and GDB
- **tmux-aware** debug sessions for optimal workflow
- **LED reaction game** Demonstrating GPIO control, EXTI, state handeling and time measuring.

## Prerequisites

- **ARM GCC toolchain**: `arm-none-eabi-gcc` and `arm-none-eabi-g++`
- **OpenOCD**: For debugging and flashing
- **st-flash**: For direct flashing (optional)
- **tmux**: For enhanced debugging experience (optional)

No STM32Cube, CubeMX or CubeIDE installation is required. The CMSIS headers this
project needs are vendored under `third_party/`, so a fresh clone builds with
nothing but the ARM toolchain and CMake.

### Installation on macOS

```bash
# Install ARM toolchain
brew install arm-none-eabi-gcc

# Install OpenOCD
brew install openocd

# Install st-flash (optional)
pip install stm32loader

# Install tmux (optional)
brew install tmux
```

## Project Structure

```
STM32L4R5ZI-Template/
├── main.cpp                 # Main C++ application
├── system_stm32l4xx.c       # STM32 system configuration
├── startup_stm32l4r5xx.s    # ARM startup assembly
├── stm32f446.ld           # Linker script
├── third_party/             # Vendored CMSIS headers (see third_party/README.md)
│   ├── cmsis-core/Include/      # CMSIS-Core (Cortex-M) headers
│   └── cmsis-device-l4/Include/ # STM32L4 device headers
├── CMakeLists.txt           # CMake build configuration
├── arm-none-eabi-toolchain.cmake  # ARM cross-compilation setup
├── Makefile                 # Convenience wrapper
└── README.md                # This file
```

## Design Contract

Nothing is configured implicitly. The template brings the chip up and enters `main()`;
everything beyond that is explicit.

- `SystemInit` does not configure the PLL and enables no peripheral clocks. It compiles to
  an empty body, so `main()` is entered on the default MSI clock (4 MHz).
- Peripheral clocks are enabled by the user in `RCC` — see `enable_port_b()` in `main.cpp`.
- GPIO pins are configured by the user; none are initialised beforehand.
- The vendored CMSIS headers are declarations only — register layouts, bit masks and
  addresses. They emit no code.

## Building

### Quick Start

```bash
# Build the project
make

# Clean build files
make clean

# Show available targets
make help
```

### Build Process

The project uses CMake for the build system with a Makefile wrapper for convenience:

1. **CMake** handles cross-compilation and linking
2. **Makefile** provides simple commands for common tasks
3. **ARM tool-chain** compiles C++, C, and Assembly files
4. **Output**: `build/stm32_reaction_game.elf` and `build/stm32_reaction_game.bin`

## Flashing

### Using st-flash (Recommended)

```bash
# Flash to STM32
make flash
```

### Using OpenOCD

```bash
# Start OpenOCD server
make openocd

# In another terminal, flash using GDB
make gdb
```

## Debugging

### Quick Debug Session

```bash
# Start full debug session (tmux-aware)
make debug
```

This will:
- **In tmux**: Create a new window with OpenOCD and GDB
- **Outside tmux**: Run OpenOCD in background, then GDB

### Manual Debugging

```bash
# Terminal 1: Start OpenOCD
make openocd

# Terminal 2: Start GDB
make gdb
```

### Debug Features

- **Automatic breakpoint** at `main()` function
- **TUI mode** enabled for better debugging experience
- **Program loading** and reset handling
- **Remote debugging** via OpenOCD

## Hardware Setup

### STM32L4R5ZI Nucleo-144

- **LED2**: Connected to GPIO Port C, Pin 4
- **Debug interface**: ST-Link V2-1
- **UART**: Available via ST-Link (USART2)

### Pin Configuration

The code Prints game instructions over UART with a baud rate of 115200
Activates the on board user LED (LED2) and after a random backoff delay disables it.
After which the user is asked to press the External button connected to PC4, triggering an interrupt that measures the reaction time of the user.
Restart of the game after 1.5 seconds of delay.

## Available Make Targets

| Target | Description |
|--------|-------------|
| `make` or `make all` | Build the project |
| `make clean` | Clean build files |
| `make flash` | Flash to STM32 using st-flash |
| `make debug` | Start debug session (tmux-aware) |
| `make openocd` | Start OpenOCD server |
| `make gdb` | Start GDB client |
| `make help` | Show available targets |

## Troubleshooting

### Common Issues

1. **"cannot find entry symbol Reset_Handler"**
   - Ensure all source files are properly included in CMakeLists.txt
   - Check that startup file is compiled as Assembly

2. **OpenOCD connection issues**
   - Verify ST-Link is connected and recognized
   - Check USB permissions on Linux

3. **Build errors**
   - Ensure ARM toolchain is properly installed
   - Verify CMake version >= 3.16

## License

This template is MIT licensed — see [LICENSE](LICENSE). The third-party files below
keep their own licenses.

Linkerscript, startup & system files are taken from ST repositories:

- [STM32F446RETX_FLASH.ld](https://github.com/STMicroelectronics/STM32CubeF4/blob/master/Projects/STM32446E-Nucleo/Templates/STM32CubeIDE/STM32F446RETX_FLASH.ld)
- [startup_stm32f446xx.s](https://github.com/STMicroelectronics/cmsis-device-f4/blob/v2.6.9/Source/Templates/gcc/startup_stm32f446xx.s) — cmsis-device-l4 `v1.7.5`, Apache-2.0
- [startup_stm32f446xx.c](https://github.com/STMicroelectronics/cmsis-device-f4/blob/v2.6.9/Source/Templates/system_stm32f4xx.c) — cmsis-device-l4 `v1.7.5`, Apache-2.0

CMSIS headers are vendored under `third_party/` as unmodified upstream files, trimmed
to the subset this target can reach:

| Package | Upstream | Tag | License |
|---------|----------|-----|---------|
| `cmsis-core` | [STMicroelectronics/cmsis-core](https://github.com/STMicroelectronics/cmsis-core) | `v5.6.0_cm4` | Apache-2.0 |
| `cmsis-device-l4` | [STMicroelectronics/cmsis-device-l4](https://github.com/STMicroelectronics/cmsis-device-l4) | `v1.7.5` | Apache-2.0 |

Each package keeps its upstream license file next to the headers. See
[third_party/README.md](third_party/README.md) for details and refresh instructions.

## References

- [STM32L446 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [Nucleo 144 User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [STM32F446xC/E Datasheet](https://www.st.com/resource/en/datasheet/stm32f446re.pdf)
- [ARM GCC Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain)
- [OpenOCD Documentation](http://openocd.org/doc/html/index.html)
