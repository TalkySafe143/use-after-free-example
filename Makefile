# Telemetry use-after-free demo.
#
# Targets:
#   make            -> builds the plain (release-ish) binary: build/telemetry
#   make asan       -> builds the ASan-instrumented binary:   build/telemetry_asan
#   make run        -> runs the plain binary
#   make run-asan   -> runs the ASan binary (this is the one that reports the bug)
#   make clean      -> removes build artifacts

CXX      := g++
CXXSTD   := -std=c++17
WARN     := -Wall -Wextra
INCLUDES := -Iinclude

SRC := src/event_bus.cpp \
       src/sensor.cpp \
       src/aggregator.cpp \
       src/device_manager.cpp \
       src/main.cpp

BUILD := build

# Plain build. -O2 so the latent bug behaves like it would in a real build:
# it may appear to "work" by reading stale-but-still-mapped memory.
PLAIN_FLAGS := $(CXXSTD) $(WARN) -O2 -g $(INCLUDES)

# AddressSanitizer build. -O1 + -fno-omit-frame-pointer gives clean, readable
# stack traces. -fsanitize=address turns on the instrumentation.
ASAN_FLAGS := $(CXXSTD) $(WARN) -O1 -g -fno-omit-frame-pointer \
              -fsanitize=address $(INCLUDES)

.PHONY: all asan run run-asan clean

all: $(BUILD)/telemetry

asan: $(BUILD)/telemetry_asan

$(BUILD)/telemetry: $(SRC) | $(BUILD)
	$(CXX) $(PLAIN_FLAGS) $(SRC) -o $@

$(BUILD)/telemetry_asan: $(SRC) | $(BUILD)
	$(CXX) $(ASAN_FLAGS) $(SRC) -o $@

$(BUILD):
	mkdir -p $(BUILD)

run: $(BUILD)/telemetry
	./$(BUILD)/telemetry

run-asan: $(BUILD)/telemetry_asan
	ASAN_OPTIONS=abort_on_error=0:halt_on_error=1 ./$(BUILD)/telemetry_asan

clean:
	rm -rf $(BUILD)
