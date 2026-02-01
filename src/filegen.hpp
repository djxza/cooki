#pragma once

#include <filesystem>
#include <iostream>
#include <print>

#include "utils.hpp"

namespace fs = std::filesystem;

class Filegen {
private:
  std::string _arg0;
  fs::path _template_dir;

  // Helper to construct template path for any target
  fs::path get_template_path_for(const fs::path &target) const {
    return _template_dir / (target.filename().string() + ".template");
  }

public:
  Filegen(const std::string &arg0) : _arg0(arg0) {
    const fs::path exe_path(_arg0);
    _template_dir = exe_path.has_parent_path() ? exe_path.parent_path()
                                               : fs::current_path();
    _template_dir /= "../templates/";

    AASSERT(fs::exists(_template_dir), "Template directory does not exist: %s",
            _template_dir.c_str());
  }

  inline void g_file(std::string_view file) {
    AASSERT(!file.empty(), "Target file path cannot be empty");

    const fs::path target_path(file);
    const fs::path template_path = get_template_path_for(target_path);

    // Validate template exists
    AASSERT(fs::exists(template_path),
            "Template '%s' not found. Expected at: %s",
            template_path.filename().c_str(), template_path.c_str());

    // Determine if we're copying a directory or file
    const bool is_directory = fs::is_directory(template_path);
    const auto copy_options =
        is_directory
            ? fs::copy_options::recursive | fs::copy_options::update_existing
            : fs::copy_options::update_existing;

    // Log what we're doing
    // for now
    // TODO: remove zis?
    // looks kinda childish idk
    std::println("📋 {} template '{}' -> '{}'",
                 is_directory ? "Directory" : "File",
                 template_path.filename().string(), target_path.string());

    // Ensure parent directory exists for target
    if (target_path.has_parent_path()) {
      fs::create_directories(target_path.parent_path());
    }

    // Perform the copy
    fs::copy(template_path, target_path, copy_options);

    // Verify success
    AASSERT(fs::exists(target_path), "Copy failed! Destination not created: %s",
            target_path.c_str());

    std::println("✅ Successfully applied template to: {}",
                 target_path.string());
  }

  inline void gen_files(const std::vector<std::string> &files) {
    for (const auto &file : files) {
      g_file(file);
    }
  }

  std::string get_arg0() const { return _arg0; }
  std::string get_template_dir() const { return _template_dir.string(); }
};
