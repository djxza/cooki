#include "lib.hpp"
#include "utils.hpp"

LibFilegen::LibFilegen(const std::string &arg0) : BaseFilegen(arg0, "lib") {}

void LibFilegen::generate_header_file(const std::string &inc_dir,
                                      const std::string &project_name,
                                      bool is_cpp,
                                      const std::string &description,
                                      const std::string &author) {
  std::string header_ext = is_cpp ? ".hpp" : ".h";
  std::string project_inc_dir = inc_dir + "/" + project_name;
  std::string header_file = project_inc_dir + "/" + project_name + header_ext;

  // Create the project-specific include directory
  fs::create_directories(project_inc_dir);

  // Generate the header content
  std::string guard = project_name + "_" + (is_cpp ? "HPP" : "H");
  std::transform(guard.begin(), guard.end(), guard.begin(), ::toupper);

  std::ostringstream header_content;
  header_content << "// " << project_name << header_ext << "\n"
                 << "// Description: " << description << "\n"
                 << "// Author: " << author << "\n"
                 << "// Created: " << __DATE__ << "\n\n"
                 << "#ifndef " << guard << "\n"
                 << "#define " << guard << "\n\n";

  if (is_cpp) {
    header_content << "#ifdef __cplusplus\n"
                   << "extern \"C\" {\n"
                   << "#endif\n\n";
  }

  header_content << "// Public API declarations\n"
                 << "int " << project_name << "_init();\n"
                 << "int " << project_name << "_do_something(int value);\n"
                 << "void " << project_name << "_cleanup();\n\n";

  if (is_cpp) {
    header_content << "#ifdef __cplusplus\n"
                   << "}\n"
                   << "#endif\n\n";
  }

  header_content << "#endif // " << guard << "\n";

  // Write the header file
  writefile(header_file, header_content.str());
  std::println("✅ Created header file: {}", header_file);
}

void LibFilegen::generate_source_file(const std::string &src_dir,
                                      const std::string &project_name,
                                      bool is_cpp) {
  std::string ext = is_cpp ? ".cpp" : ".c";
  std::string src_file = src_dir + "/" + project_name + ext;

  // Generate the source content
  std::ostringstream src_content;
  src_content << "#include \"" << project_name << "/" << project_name
              << (is_cpp ? ".hpp" : ".h") << "\"\n\n"
              << "#include <stdio.h>\n\n"
              << "int " << project_name << "_init() {\n"
              << "    printf(\"" << project_name
              << " library initialized\\n\");\n"
              << "    return 0;\n"
              << "}\n\n"
              << "int " << project_name << "_do_something(int value) {\n"
              << "    return value * 2;\n"
              << "}\n\n"
              << "void " << project_name << "_cleanup() {\n"
              << "    printf(\"" << project_name
              << " library cleaned up\\n\");\n"
              << "}\n";

  // Write the source file
  writefile(src_file, src_content.str());
  std::println("✅ Created source file: {}", src_file);
}

void LibFilegen::generate_makefile(const std::string &project_name) {
  g_file("makefile");
}

void LibFilegen::generate_test_directory(const std::string &project_name,
                                         bool is_cpp) {
  std::string test_dir = "test";
  std::string test_makefile = test_dir + "/makefile";
  std::string test_src_file = test_dir + "/test" + (is_cpp ? ".cpp" : ".c");

  // Create test directory
  fs::create_directories(test_dir);

  // Generate test makefile
  std::ostringstream makefile_content;
  makefile_content
      << "# "
         "====================================================================="
         "=======\n"
      << "# Test Makefile for " << project_name << " library\n"
      << "# "
         "====================================================================="
         "=======\n\n"
      << "# Project configuration\n"
      << "PROJECT_NAME := " << project_name << "\n"
      << "LANG := " << (is_cpp ? "C++" : "C") << "\n\n"
      << "# Directories\n"
      << "SRC_DIR := ../src\n"
      << "INC_DIR := ../include\n"
      << "LIB_DIR := ../lib\n"
      << "BIN_DIR := ./bin\n\n"
      << "# Compiler configuration\n"
      << "ifeq ($(LANG),C)\n"
      << "    CC := gcc\n"
      << "    STD := c17\n"
      << "else\n"
      << "    CC := g++\n"
      << "    STD := c++26\n"
      << "endif\n\n"
      << "# Source files\n"
      << "TEST_SRC := test" << (is_cpp ? ".cpp" : ".c") << "\n"
      << "TARGET := $(BIN_DIR)/test_$(PROJECT_NAME)\n\n"
      << "# Compiler flags\n"
      << "CFLAGS := -std=$(STD) -Wall -Wextra -Wpedantic -g \\\n"
      << "          -I$(INC_DIR)/$(PROJECT_NAME) -I$(INC_DIR)\n"
      << "LDFLAGS := -L$(LIB_DIR) -l$(PROJECT_NAME) -Wl,-rpath,$(LIB_DIR)\n\n"
      << "# Phony targets\n"
      << ".PHONY: all clean run\n\n"
      << "# Default target\n"
      << "all: dirs $(TARGET)\n\n"
      << "# Create directories\n"
      << "dirs:\n"
      << "	@mkdir -p $(BIN_DIR)\n\n"
      << "# Build test executable\n"
      << "$(TARGET): $(TEST_SRC) $(LIB_DIR)/lib$(PROJECT_NAME).so | dirs\n"
      << "	@echo \"🧪 Building test executable...\"\n"
      << "	@$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@\n"
      << "	@echo \"✅ Built test executable\"\n\n"
      << "# Run tests\n"
      << "run: $(TARGET)\n"
      << "	@echo \"🚀 Running tests...\"\n"
      << "	@LD_LIBRARY_PATH=$(LIB_DIR):$$LD_LIBRARY_PATH ./$(TARGET)\n\n"
      << "# Clean\n"
      << "clean:\n"
      << "	@echo \"🧹 Cleaning test artifacts...\"\n"
      << "	@rm -rf $(BIN_DIR)\n\n"
      << "# Help\n"
      << "help:\n"
      << "	@echo \"Test Makefile for $(PROJECT_NAME) library\"\n"
      << "	@echo \"Targets:\"\n"
      << "	@echo \"  all    - Build test executable (default)\"\n"
      << "	@echo \"  run    - Build and run tests\"\n"
      << "	@echo \"  clean  - Remove test binaries\"\n"
      << "	@echo \"  help   - Show this help\"\n";

  writefile(test_makefile, makefile_content.str());
  std::println("✅ Created test makefile: {}", test_makefile);

  // Generate test source file
  std::ostringstream test_content;
  test_content << "#include \"" << project_name << "/" << project_name
               << (is_cpp ? ".hpp" : ".h") << "\"\n"
               << "#include <stdio.h>\n\n"
               << "int main() {\n"
               << "    printf(\"Testing " << project_name << " library\\n\");\n"
               << "    \n"
               << "    if (" << project_name << "_init() != 0) {\n"
               << "        printf(\"Failed to initialize library\\n\");\n"
               << "        return 1;\n"
               << "    }\n"
               << "    \n"
               << "    int result = " << project_name << "_do_something(21);\n"
               << "    printf(\"Result: %d\\n\", result);\n"
               << "    \n"
               << "    " << project_name << "_cleanup();\n"
               << "    return 0;\n"
               << "}\n";

  writefile(test_src_file, test_content.str());
  std::println("✅ Created test source file: {}", test_src_file);
}
