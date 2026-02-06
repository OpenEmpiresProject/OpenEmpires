#include <gtest/gtest.h>

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

class PythonTestEnvironment : public ::testing::Environment
{
  public:
    void SetUp() override
    {
        py::initialize_interpreter();
        py::module sys = py::module::import("sys");
        py::list path = sys.attr("path");
        path.append(PYTHON_MODEL_DIR);
        path.append(TEST_PYTHON_MODEL_DIR);
    }

    void TearDown() override
    {
    }
};

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new PythonTestEnvironment());
    return RUN_ALL_TESTS();
}