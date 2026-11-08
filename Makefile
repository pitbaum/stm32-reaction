# Build directory
BUILD_DIR = build

# Project name (same as in CMakeLists.txt)
PROJECT_NAME = stm32_reaction_game

# Default target
all: $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake -DCMAKE_TOOLCHAIN_FILE=../arm-none-eabi-toolchain.cmake ..

clean:
	rm -rf $(BUILD_DIR)

flash: all
	st-flash write $(BUILD_DIR)/$(PROJECT_NAME).bin 0x08000000

# Debug targets
debug: all
	@if [ -n "$$TMUX" ]; then \
		echo "Starting debug session in tmux..."; \
		tmux split-window -h "openocd -f interface/stlink.cfg -f target/stm32l4x.cfg"; \
		sleep 2; \
		tmux new-window "arm-none-eabi-gdb $(BUILD_DIR)/$(PROJECT_NAME).elf \
			-ex 'target remote localhost:3333' \
			-ex 'monitor halt' \
			-ex 'monitor reset halt' \
			-ex 'load' \
			-ex 'break main' \
			-ex 'continue' \
			-ex 'tui enable'"; \
	else \
		echo "Not in tmux session. Starting OpenOCD in background..."; \
		openocd -f interface/stlink.cfg -f target/stm32l4x.cfg & \
		sleep 2; \
		arm-none-eabi-gdb $(BUILD_DIR)/$(PROJECT_NAME).elf \
			-ex 'target remote localhost:3333' \
			-ex 'monitor halt' \
			-ex 'monitor reset halt' \
			-ex 'load' \
			-ex 'break main' \
			-ex 'continue'; \
		kill %1; \
	fi

openocd:
	openocd -f interface/stlink.cfg -f target/stm32l4x.cfg

gdb: all
	arm-none-eabi-gdb $(BUILD_DIR)/$(PROJECT_NAME).elf \
		-ex 'target remote localhost:3333' \
		-ex 'monitor halt' \
		-ex 'monitor reset halt' \
		-ex 'load' \
		-ex 'monitor reset halt' \
		-ex 'break main' \
		-ex 'continue'

help:
	@echo "Available targets:"
	@echo "  all     - Build the project (default)"
	@echo "  build   - Build binary file"
	@echo "  flash   - Flash binary to STM32"
	@echo "  clean   - Clean build files"
	@echo "  debug   - Start debug session (tmux-aware)"
	@echo "  openocd - Start OpenOCD server"
	@echo "  gdb     - Start GDB client (requires OpenOCD running)"
	@echo "  help    - Show this help"

.PHONY: all build flash clean debug openocd gdb help
