# Directories
BUILD_DIR := build
GAME_EXEC := openEmpires
TEST_EXEC := openEmpiresTests
INTEG_TEST_EXEC := openEmpiresIntegTests

# Targets
.PHONY: configure build test integ-test run clean

# Default target
.PHONY: default
default: build

# Configure the project using CMake (out-of-source build)
configure:
	@echo "Configuring project..."
# 	Following requires --preset=vcpkg, otherwise it will fail to identify package configuration files
#   Following requires -G "Visual Studio 17 2022", otherwise ninja will fail to locate standard include files
	cmake --preset=vcpkg -G "Visual Studio 17 2022" -A x64 -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Debug

# Configure Release build
configure-release:
	@echo "Configuring project (Release)..."
# Using static libraries for the releases 
	cmake --preset=vcpkg -G "Visual Studio 17 2022" -A x64 -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=x64-windows

# Build the project
build:
	@echo "Building project..."
	cmake --build $(BUILD_DIR)

build-verbose:
	@echo "Building project..."
	cmake --build $(BUILD_DIR) --verbose

# Build Release
build-release: configure-release
	@echo "Building project (Release)..."
	cmake --build $(BUILD_DIR) --config Release

# Format the source code using clang-format
format:
	@echo "Formatting sources..."
	cmake --build $(BUILD_DIR) --target format

	python normalize_line_endings.py

# Analyze the source code using cppcheck
cppcheck:
	@echo "Cppchecking sources..."
	cmake --build $(BUILD_DIR) --target cppcheck
	
# Run the tests (directly execute the test binary)
test: build
	@echo Running tests...
ifeq ($(strip $(FILTER)),)
	@$(BUILD_DIR)\bin\Debug\$(TEST_EXEC).exe
else
	@$(BUILD_DIR)\bin\Debug\$(TEST_EXEC).exe --gtest_filter=$(FILTER)
endif

# Run the integration tests (directly execute the test binary)
integ-test: build
	@echo "Running integration tests..."
ifeq ($(strip $(FILTER)),)
	$(BUILD_DIR)/bin/Debug/$(INTEG_TEST_EXEC)
else
	$(BUILD_DIR)/bin/Debug/$(INTEG_TEST_EXEC) --gtest_filter=$(FILTER)
endif

# Run the game (directly execute the game binary)
run: build
	@echo "Running the game..."
	$(BUILD_DIR)/bin/Debug/$(GAME_EXEC)

run-release: release
	@echo "Running the game..."
	$(BUILD_DIR)/bin/Release/$(GAME_EXEC)

# Clean the build directory
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)

# Run formatting, unit tests, and integration tests
.PHONY: pre-commit
pre-commit: format test integ-test
	@echo "All checks passed. Ready to commit!"
