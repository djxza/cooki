#include "exe.hpp"

ExeFilegen::ExeFilegen(const std::string &arg0) : BaseFilegen(arg0, "exe") {}

void ExeFilegen::generate_main_file(const std::string &src_dir,
                                    const std::string &project_name,
                                    bool is_cpp) {
  std::string ext = is_cpp ? ".cpp" : ".c";
  std::string main_file = src_dir + "/main" + ext;
  g_file(main_file);
}

void ExeFilegen::generate_makefile(const std::string &project_name) {
  g_file("makefile");
}
