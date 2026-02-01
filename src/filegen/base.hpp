#pragma once

#include <filesystem>
#include <iostream>
#include <print>
#include <string>
#include <vector>

#include "utils.hpp"

namespace fs = std::filesystem;

class BaseFilegen {
protected:
  std::string _arg0;
  fs::path _template_dir;

  // Helper to construct template path for any target
  fs::path get_template_path_for(const fs::path &target) const {
    return _template_dir / (target.filename().string() + ".template");
  }

public:
  BaseFilegen(const std::string &arg0, const std::string &template_subdir = "");
  virtual ~BaseFilegen() = default;

  virtual void g_file(std::string_view file);
  virtual void gen_files(const std::vector<std::string> &files);

  std::string get_arg0() const { return _arg0; }
  std::string get_template_dir() const { return _template_dir.string(); }
};
