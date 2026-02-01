#pragma once

#include "base_filegen.hpp"
#include <sstream>

class LibFilegen : public BaseFilegen {
public:
  LibFilegen(const std::string &arg0);

  void generate_header_file(const std::string &inc_dir,
                            const std::string &project_name, bool is_cpp,
                            const std::string &description,
                            const std::string &author);
  void generate_source_file(const std::string &src_dir,
                            const std::string &project_name, bool is_cpp);
  void generate_makefile(const std::string &project_name);
  void generate_test_file(const std::string &src_dir,
                          const std::string &project_name, bool is_cpp);
};
