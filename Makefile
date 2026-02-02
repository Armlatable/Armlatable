
# Makefile
# Arduino CLI & CMake Wrapper

# --- Configuration ---
PORT_DONGLE ?= /dev/tty.usbserial-1420
PORT_ROBOT  ?= /dev/tty.usbmodemF412FA77615C2

FQBN_ROBOT  = arduino:renesas_uno:unor4wifi
FQBN_DONGLE = esp32:esp32:esp32
# Note: FQBN for Dongle needs to be verified based on exact board (Seeed vs Generic)

# Directories
BUILD_DIR_HOST = host/build
MODULE_Library = $(PWD)/libraries

# Python Interpreter (Use venv if available)
PYTHON ?= /Users/somadayon/Armlatable/venv/bin/python3

# --- Host Targets ---
.PHONY: all setup clean run-main run-viewer generate-config

generate-config:
	$(PYTHON) scripts/generate_config_header.py config.yaml module/src/configuration/configuration.h

setup: generate-config
	@echo ">> Setting up Host CMake..."
	mkdir -p $(BUILD_DIR_HOST)
	cd $(BUILD_DIR_HOST) && cmake ..
	@echo ">> Installing Arduino Cores..."
	arduino-cli core update-index
	arduino-cli core install arduino:renesas_uno
	# arduino-cli core install esp32:esp32 # Uncomment if needed (heavy)

build-host: generate-config
	cmake --build $(BUILD_DIR_HOST)

run-main: build-host
	$(BUILD_DIR_HOST)/locomo_cli $(PORT_DONGLE)

run-viewer: build-host
	$(BUILD_DIR_HOST)/locomo_viewer

# --- Firmware Targets ---
.PHONY: flash flash-dongle

flash: generate-config
	@echo ">> Flashing Robot (R4 WiFi) at $(PORT_ROBOT)"
	@# Copy protocol and configuration to sketch folder
	cp -r module/src/protocol firmware/robot_r4_wifi/
	cp -r module/src/configuration firmware/robot_r4_wifi/
	arduino-cli compile --fqbn $(FQBN_ROBOT) firmware/robot_r4_wifi
	arduino-cli upload -p $(PORT_ROBOT) --fqbn $(FQBN_ROBOT) firmware/robot_r4_wifi
	@# Cleanup
	rm -rf firmware/robot_r4_wifi/protocol firmware/robot_r4_wifi/configuration

flash-dongle: generate-config
	@echo ">> Flashing Dongle (ESP32) at $(PORT_DONGLE)"
	@# Copy protocol and configuration to sketch folder
	cp -r module/src/protocol firmware/dongle_esp/
	cp -r module/src/configuration firmware/dongle_esp/
	arduino-cli compile --fqbn $(FQBN_DONGLE) firmware/dongle_esp
	arduino-cli upload -p $(PORT_DONGLE) --fqbn $(FQBN_DONGLE) firmware/dongle_esp
	rm -rf firmware/dongle_esp/protocol firmware/dongle_esp/configuration

flash-test: generate-config
	@echo ">> Flashing Test Firmware (R4 WiFi) at $(PORT_ROBOT)"
	@# Copy protocol and configuration to sketch folder
	cp -r module/src/protocol firmware/test/wifi_comm_test/
	cp -r module/src/configuration firmware/test/wifi_comm_test/
	arduino-cli compile --fqbn $(FQBN_ROBOT) firmware/test/wifi_comm_test
	arduino-cli upload -p $(PORT_ROBOT) --fqbn $(FQBN_ROBOT) firmware/test/wifi_comm_test
	rm -rf firmware/test/wifi_comm_test/protocol firmware/test/wifi_comm_test/configuration

clean:
	rm -rf $(BUILD_DIR_HOST)
