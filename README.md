# openEmpires - README.md

# openEmpires

openEmpires is a cross-platform clone of Age of Empires 2, designed to provide a similar gameplay experience while allowing for modern enhancements and modifications. This project is built using C++ for the core game engine and Python for scripting and game logic.

## Features (Planned)

- Real-time strategy gameplay
- Single-player and multiplayer modes
- Resource management and building mechanics
- Unit control and combat
- Modifiable game logic through Python scripting

## Getting Started

### Prerequisites

- CMake (version 3.10 or higher)
- A C++ compiler (GCC, Clang, or MSVC) - Tested against MSVC 19
- Python 3.x
- Make
- vcpkg (for C++ package management)
- clang-format

> Note: If required clone and setup vcpkg manually to get the latest package list.

#### Dependencies
- SDL3
- SDL3-image
- GTest
- EnTT (for entity-component-system)
- spdlog (for logging)

#### Setting env variables
- Add path to CMake to the PATH variable
- Add path to C++ compiler to the PATH variable
- Add path to vcpkg to the PATH variable
- Define VCPKG_ROOT env variable which points to the vcpkg path

An example env setup in Windows might look like this;
```
$vsBase = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
$vcpkgPath = "E:\Projects\openEmpires\vcpkg\vcpkg"

$env:Path += ";$vsBase\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
$env:Path += ";$vsBase\VC\Tools\MSVC\14.43.34808\bin\Hostx86\x64"
$env:Path += ";$vcpkgPath"
$env:VCPKG_ROOT = $vcpkgPath

```

### Building the Project

1. Clone the repository:

   ```
   git clone https://github.com/OpenEmpiresProject/OpenEmpires.git
   cd openEmpires
   ```

2. Configure the project using CMake:

   ```
   make configure
   ```

3. Build the project:

   ```
   make
   ```

### Running the Game

   ```
   make run
   ```
### Running Tests

   ```
   make test
   ```
## Contributing

We’d love your help! There are plenty of ways to contribute, and we appreciate every effort to improve the project. Please fork the repository and submit a pull request with your changes.

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgments

- Inspired by Age of Empires 2
- Thanks to the open-source community for their contributions and support.