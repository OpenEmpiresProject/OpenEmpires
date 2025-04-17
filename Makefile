# Directories
BUILD_DIR := build
TEST_DIR := tests
GAME_EXEC := openEmpires  # Replace with your game's executable name
TEST_EXEC := openEmpiresTests  # Replace with your test executable name

# Targets
.PHONY: configure build test run clean

# Default target
.PHONY: default
default: build

# Configure the project using CMake (out-of-source build)
configure:
	@echo "Configuring project..."
# 	Following requires --preset=vcpkg, otherwise it will fail to identify package configuration files
#   Following requires -G "Visual Studio 17 2022", otherwise ninja will fail to locate standard include files
	cmake --preset=vcpkg -G "Visual Studio 17 2022" -A x64 -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Debug

# Build the project
build:
	@echo "Building project..."
	cmake --build $(BUILD_DIR)

build-verbose:
	@echo "Building project..."
	cmake --build $(BUILD_DIR) --verbose
	
# Run the tests (directly execute the test binary)
test: build
	@echo "Running tests..."
	$(BUILD_DIR)/bin/Debug/$(TEST_EXEC)

# Run the game (directly execute the game binary)
run: build
	@echo "Running the game..."
	$(BUILD_DIR)/bin/Debug/$(GAME_EXEC)

# Clean the build directory
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)
