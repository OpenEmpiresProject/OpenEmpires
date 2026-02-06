#include <pybind11/embed.h>

// Ensure the embedded Python interpreter is initialized for the whole process.
// This prevents destruction of pybind11-managed Python objects after the interpreter
// has been finalized which causes "pybind11::handle::dec_ref() ... GIL is not held" errors.
//
// The file is intentionally minimal: constructing a single global py::scoped_interpreter
// guarantees the interpreter (and the GIL) exist for the entire program lifetime.
//static pybind11::scoped_interpreter s_globalPythonInterpreter{};

class PythonInitializer
{
  public:
    static void init()
    {
        static pybind11::scoped_interpreter guard{};
    }
};