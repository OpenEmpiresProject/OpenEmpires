set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PYTHON_EXECUTABLE /usr/bin/python3)
set(PYTHON_INCLUDE_DIR /usr/include/python3.x)
set(PYTHON_LIBRARY /usr/lib/x86_64-linux-gnu/libpython3.xm)

include(FindPython3)