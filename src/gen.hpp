#pragma once

#include "project.hpp"
#include "utils.hpp"

class Filegen {
private:
  std::string _arg0;

public:
  Filegen(const std::string &arg0) : _arg0(arg0) {}

  inline void gen_makefile(const Project &p) {
    const std::string &contents = readfile(
        _arg0.substr(0, _arg0.find_last_of('/')) + "/makefile.template");
    if (p.)
      contents.insert(contents.find_first_of("CC = ") + 5, "clang++");

    std::cout << contents << std::endl;
  }
};
