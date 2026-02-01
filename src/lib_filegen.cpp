#include "lib_filegen.hpp"
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

void LibFilegen::generate_test_file(const std::string &src_dir,
                                    const std::string &project_name,
                                    bool is_cpp) {
  std::string ext = is_cpp ? ".cpp" : ".c";
  std::string test_file = src_dir + "/test" + ext;

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

  writefile(test_file, test_content.str());
  std::println("✅ Created test file: {}", test_file);
}
