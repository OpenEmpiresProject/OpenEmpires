import os
import sys

def create_class_files(namespace: str, class_name: str):
    # Base folder
    base_dir = os.path.join("src", "cpp", namespace)
    os.makedirs(base_dir, exist_ok=True)

    # File paths
    header_file = os.path.join(base_dir, f"{class_name}.h")
    cpp_file = os.path.join(base_dir, f"{class_name}.cpp")

    # Include guard (uppercase with underscores)
    guard = f"{namespace.upper()}_{class_name.upper()}_H"

    # Header file content
    header_content = f"""#ifndef {guard}
#define {guard}

namespace {namespace}
{{
class {class_name}
{{
public:
    {class_name}();
    ~{class_name}();

private:

}};
}} // namespace {namespace}

#endif // {guard}
"""

    # CPP file content
    cpp_content = f"""#include "{class_name}.h"

using namespace {namespace};

{class_name}::{class_name}()
{{
    // constructor
}}

{class_name}::~{class_name}()
{{
    // destructor
}}
"""

    # Write files
    with open(header_file, "w", encoding="utf-8") as h:
        h.write(header_content)

    with open(cpp_file, "w", encoding="utf-8") as cpp:
        cpp.write(cpp_content)

    print(f"Created {header_file} and {cpp_file}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python create_class.py <namespace> <ClassName>")
        sys.exit(1)

    namespace = sys.argv[1]
    class_name = sys.argv[2]
    create_class_files(namespace, class_name)
