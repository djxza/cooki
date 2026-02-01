#pragma once

#include "base.hpp"

class ExeFilegen : public BaseFilegen {
public:
  ExeFilegen(const std::string &arg0);

  void generate_main_file(const std::string &src_dir,
                          const std::string &project_name, bool is_cpp);
  void generate_makefile(const std::string &project_name);
};
