#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../nlohmann/json.hpp"
#include "exe_filegen.hpp"
#include "lib_filegen.hpp"
#include "utils.hpp"

using json = nlohmann::ordered_json;

struct Version {
  u32 major, minor, patch;
  std::string prerelease;
  std::string build;

  Version();
  Version(u32 _major, u32 _minor, u32 _patch = 0,
          const std::string &_prerelease = "", const std::string &_build = "");

  std::string to_string() const;
  static Version from_string(const std::string &version_str);
};

enum class ProjectLang { C, CPP };
enum class ProjectType { EXE, LIB };

class Project {
private:
  std::string name;
  ProjectType type;
  Version version;
  std::string description;
  std::string author;
  std::string license;
  ProjectLang lang;
  std::string git_repo;
  std::string compiler_path;
  std::vector<std::string> libs;

  // Library-specific fields
  bool create_test;
  std::string api_prefix;

  std::string src_dir;
  std::string lib_dir;
  std::string bin_dir;
  std::string inc_dir;
  std::vector<std::string> dependencies;

  std::unique_ptr<BaseFilegen> filegen;
  std::string arg0;

  void setup_filegen();
  void make_exe_dirs();
  void make_lib_dirs();
  void make_exe_files();
  void make_lib_files();

public:
  Project(const std::string &arg0,
          const std::string &project_data = "./project.json");
  Project(const std::string &arg0, const std::string &_name,
          const Version &_version, const std::string &_description,
          const std::string &_author, const std::string &_license,
          ProjectLang _lang, const std::string &_git_repo,
          const std::string &_compiler_path,
          const std::vector<std::string> &_libs = {});

  static std::string prompt_with_default(const std::string &prompt,
                                         const std::string &default_value,
                                         bool non_interactive = false,
                                         bool newline = true);
  static bool prompt_yes_no(const std::string &prompt, bool default_value,
                            bool non_interactive = false);

  Project _init(bool non_interactive = false);
  void init(bool non_interactive = false);
  void parse(const std::string &proj_file_path);
  void write(const std::string &proj_file_path = "project.json") const;
  void make_dirs();
  void make_files();

  // Getters
  const std::string &get_name() const { return name; }
  const Version &get_version() const { return version; }
  const std::string &get_description() const { return description; }
  const std::string &get_author() const { return author; }
  const std::string &get_license() const { return license; }
  ProjectLang get_lang() const { return lang; }
  ProjectType get_type() const { return type; }
  const std::string &get_git_repo() const { return git_repo; }
  const std::string &get_compiler_path() const { return compiler_path; }
  const std::string &get_src_dir() const { return src_dir; }
  const std::string &get_lib_dir() const { return lib_dir; }
  const std::string &get_bin_dir() const { return bin_dir; }
  const std::string &get_inc_dir() const { return inc_dir; }
  const std::vector<std::string> &get_libs() const { return libs; }
  const std::vector<std::string> &get_dependencies() const {
    return dependencies;
  }
  std::string get_ext() const;
  bool get_create_test() const { return create_test; }
  const std::string &get_api_prefix() const { return api_prefix; }

  // Setters
  void set_name(const std::string &value) { name = value; }
  void set_version(const Version &value) { version = value; }
  void set_description(const std::string &value) { description = value; }
  void set_author(const std::string &value) { author = value; }
  void set_license(const std::string &value) { license = value; }
  void set_lang(ProjectLang value) { lang = value; }
  void set_type(ProjectType value);
  void set_git_repo(const std::string &value) { git_repo = value; }
  void set_compiler_path(const std::string &value) { compiler_path = value; }
  void set_src_dir(const std::string &value) { src_dir = value; }
  void set_lib_dir(const std::string &value) { lib_dir = value; }
  void set_bin_dir(const std::string &value) { bin_dir = value; }
  void set_inc_dir(const std::string &value) { inc_dir = value; }
  void set_libs(const std::vector<std::string> &value) { libs = value; }
  void set_create_test(bool value) { create_test = value; }
  void set_api_prefix(const std::string &value) { api_prefix = value; }

  void add_lib(const std::string &lib) { libs.push_back(lib); }
  void add_dependency(const std::string &dep) { dependencies.push_back(dep); }
};
