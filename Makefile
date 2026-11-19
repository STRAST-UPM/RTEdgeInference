.DEFAULT_GOAL := help

PYTHON ?= python3
IDF_PY ?= idf.py
PLATFORM ?= esp32
PORT ?=
BAUD ?= 115200

ESP32_HW_MODEL ?= ESP32_HW_MODEL_ESP32
STM32_HW_MODEL ?= STM32_HW_MODEL_H743ZI2
STM32_OPENOCD_IFACE ?= interface/stlink.cfg
STM32_OPENOCD_TARGET ?= target/stm32h7x.cfg
STM32_MONITOR_SECS ?= 0
STM32_MONITOR_LINES ?= 0

CONFIG_FILE ?= examples/config.yaml
MODEL_DIR ?= examples/models
EVENTS_FILE ?= examples/data/events_serial.txt

EDGE_ROOT := $(abspath edge)
BUILD_DIR := build/$(PLATFORM)
GEN_DIR := $(BUILD_DIR)/generated
ESP32_PROJECT_DIR := build/esp32/idf-project
ESP32_BUILD_DIR := build/esp32/idf-build
STM32_BUILD_DIR := build/stm32/firmware
STM32_ELF := $(STM32_BUILD_DIR)/RTEdgeInference-STM32.elf
TFLM_DIR ?= $(abspath $(ESP32_PROJECT_DIR)/managed_components/espressif__esp-tflite-micro)

ifeq ($(PLATFORM),esp32)
else ifeq ($(PLATFORM),stm32)
else
$(error Unsupported PLATFORM='$(PLATFORM)'. Choose esp32 or stm32)
endif

.PHONY: help check-esp32 check-stm32 generate prepare-esp32-app \
        resolve-esp32-components build build-esp32 build-stm32 flash run monitor clean

help:
	@echo "Targets:"
	@echo "  make build [PLATFORM=esp32|stm32] [MODEL_DIR=...] [CONFIG_FILE=...]"
	@echo "  make flash PLATFORM=esp32 PORT=/dev/ttyUSB0"
	@echo "  make flash PLATFORM=stm32"
	@echo "  make run PLATFORM=esp32|stm32 PORT=/dev/ttyUSB0"
	@echo "  make monitor PLATFORM=esp32|stm32 PORT=/dev/ttyUSB0"
	@echo "  make generate [PLATFORM=esp32|stm32]"
	@echo "  make clean [PLATFORM=esp32|stm32]"

check-esp32:
	@command -v "$(IDF_PY)" >/dev/null 2>&1 || { echo "ERROR: idf.py is not in PATH; activate ESP-IDF first." >&2; exit 2; }
	@test -n "$$IDF_PATH" || { echo "ERROR: IDF_PATH is unset; source ESP-IDF export.sh first." >&2; exit 2; }

check-stm32:
	@for tool in cmake arm-none-eabi-gcc arm-none-eabi-g++; do \
		command -v "$$tool" >/dev/null 2>&1 || { echo "ERROR: required STM32 tool not found: $$tool" >&2; exit 2; }; \
	done

generate:
	@test -f "$(CONFIG_FILE)" || { echo "ERROR: config file not found: $(CONFIG_FILE)" >&2; exit 2; }
	@test -f "$(MODEL_DIR)/models_props.csv" || { echo "ERROR: model properties not found: $(MODEL_DIR)/models_props.csv" >&2; exit 2; }
	@test -n "$$(find "$(MODEL_DIR)" -maxdepth 1 -name '*.tflite' -print -quit)" || { echo "ERROR: no .tflite models in $(MODEL_DIR)" >&2; exit 2; }
	@test -f "$(EVENTS_FILE)" || { echo "ERROR: event sequence file not found: $(EVENTS_FILE)" >&2; exit 2; }
	@mkdir -p "$(GEN_DIR)"
	$(PYTHON) tools/gen_config.py "$(CONFIG_FILE)" "$(BUILD_DIR)/config.h"
	$(PYTHON) tools/gen_models_data.py "$(MODEL_DIR)" "$(GEN_DIR)/models_data.c" "$(GEN_DIR)/models_data.h"
	$(PYTHON) tools/gen_memory_data.py --input "$(EVENTS_FILE)" --output "$(GEN_DIR)/memory_events.h"

prepare-esp32-app:
	@mkdir -p "$(ESP32_PROJECT_DIR)/main"
	cp edge/platform/esp32/app/CMakeLists.txt "$(ESP32_PROJECT_DIR)/CMakeLists.txt"
	cp edge/platform/esp32/app/dependencies.lock "$(ESP32_PROJECT_DIR)/dependencies.lock"
	cp edge/platform/esp32/app/sdkconfig.defaults "$(ESP32_PROJECT_DIR)/sdkconfig.defaults"
	cp -R edge/platform/esp32/main/. "$(ESP32_PROJECT_DIR)/main/"

resolve-esp32-components: check-esp32 generate prepare-esp32-app
	"$(IDF_PY)" -C "$(ESP32_PROJECT_DIR)" -B "$(ESP32_BUILD_DIR)" reconfigure \
		-DEDGE_WORKSPACE_ROOT="$(EDGE_ROOT)" \
		-DEDGE_GENERATED_DIR="$(abspath $(BUILD_DIR))" \
		-DESP32_HW_MODEL="$(ESP32_HW_MODEL)" \
		-DENABLE_TRACES=1

build: build-$(PLATFORM)

build-esp32: check-esp32 generate prepare-esp32-app
	"$(IDF_PY)" -C "$(ESP32_PROJECT_DIR)" -B "$(ESP32_BUILD_DIR)" build \
		-DEDGE_WORKSPACE_ROOT="$(EDGE_ROOT)" \
		-DEDGE_GENERATED_DIR="$(abspath $(BUILD_DIR))" \
		-DESP32_HW_MODEL="$(ESP32_HW_MODEL)" \
		-DENABLE_TRACES=1

build-stm32: check-stm32 resolve-esp32-components
	@test -f "$(TFLM_DIR)/tensorflow/lite/micro/micro_interpreter.cc" || { echo "ERROR: TensorFlow Lite Micro component not found at $(TFLM_DIR)" >&2; exit 2; }
	CCACHE_DISABLE=1 cmake -S edge/platform/stm32/fw -B "$(STM32_BUILD_DIR)" \
		-DCMAKE_TOOLCHAIN_FILE="$(CURDIR)/edge/platform/stm32/fw/cmake/arm-none-eabi.cmake" \
		-DCMAKE_BUILD_TYPE=Release \
		-DENABLE_TRACES=1 \
		-DSTM32_HW_MODEL="$(STM32_HW_MODEL)" \
		-DEDGE_WORKSPACE_ROOT="$(EDGE_ROOT)" \
		-DEDGE_GENERATED_DIR="$(abspath $(BUILD_DIR))" \
		-DTFLM_DIR="$(TFLM_DIR)" \
		-DFETCHCONTENT_BASE_DIR="$(abspath $(BUILD_DIR))/fetchcontent"
	CCACHE_DISABLE=1 cmake --build "$(STM32_BUILD_DIR)" --parallel

flash: build
ifeq ($(PLATFORM),esp32)
	@test -n "$(PORT)" || { echo "ERROR: set PORT, for example PORT=/dev/ttyUSB0" >&2; exit 2; }
	"$(IDF_PY)" -C "$(ESP32_PROJECT_DIR)" -B "$(ESP32_BUILD_DIR)" -p "$(PORT)" flash
else
	@command -v openocd >/dev/null 2>&1 || { echo "ERROR: openocd is required to flash STM32." >&2; exit 2; }
	openocd -f "$(STM32_OPENOCD_IFACE)" -f "$(STM32_OPENOCD_TARGET)" \
		-c "init; program $(STM32_ELF) verify; reset run; exit"
endif

run: build
ifeq ($(PLATFORM),esp32)
	@test -n "$(PORT)" || { echo "ERROR: set PORT, for example PORT=/dev/ttyUSB0" >&2; exit 2; }
	"$(IDF_PY)" -C "$(ESP32_PROJECT_DIR)" -B "$(ESP32_BUILD_DIR)" -p "$(PORT)" flash monitor
else
	@test -n "$(PORT)" || { echo "ERROR: set PORT, for example PORT=/dev/ttyACM0" >&2; exit 2; }
	@command -v openocd >/dev/null 2>&1 || { echo "ERROR: openocd is required to run STM32." >&2; exit 2; }
	openocd -f "$(STM32_OPENOCD_IFACE)" -f "$(STM32_OPENOCD_TARGET)" \
		-c "init; program $(STM32_ELF) verify; reset halt; exit"
	@mkdir -p "$(BUILD_DIR)"
	@set -e; \
		$(PYTHON) tools/stm32_serial_monitor.py --port "$(PORT)" --baud "$(BAUD)" \
			--secs "$(STM32_MONITOR_SECS)" --lines "$(STM32_MONITOR_LINES)" \
			--log "$(BUILD_DIR)/serial.log" & \
		monitor_pid=$$!; \
		trap 'kill "$$monitor_pid" 2>/dev/null || true' EXIT INT TERM; \
		sleep 1; \
		openocd -f "$(STM32_OPENOCD_IFACE)" -f "$(STM32_OPENOCD_TARGET)" \
			-c "init; reset run; exit"; \
		wait "$$monitor_pid"; \
		trap - EXIT INT TERM
endif

monitor:
ifeq ($(PLATFORM),esp32)
	@test -n "$(PORT)" || { echo "ERROR: set PORT, for example PORT=/dev/ttyUSB0" >&2; exit 2; }
	"$(IDF_PY)" -C "$(ESP32_PROJECT_DIR)" -B "$(ESP32_BUILD_DIR)" -p "$(PORT)" monitor
else
	@test -n "$(PORT)" || { echo "ERROR: set PORT, for example PORT=/dev/ttyACM0" >&2; exit 2; }
	@$(PYTHON) -c 'import serial' || { echo "ERROR: install Python requirements with pip install -r requirements.txt" >&2; exit 2; }
	@mkdir -p "$(BUILD_DIR)"
	$(PYTHON) tools/stm32_serial_monitor.py --port "$(PORT)" --baud "$(BAUD)" \
		--secs "$(STM32_MONITOR_SECS)" --lines "$(STM32_MONITOR_LINES)" \
		--log "$(BUILD_DIR)/serial.log"
endif

clean:
	rm -rf "build/$(PLATFORM)"
