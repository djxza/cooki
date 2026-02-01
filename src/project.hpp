#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "../nlohmann/json.hpp"
#include "filegen.hpp"
#include "utils.hpp"

using json = nlohmann::ordered_json;

struct Version {
  u32 major, minor, patch;
  std::string prerelease;
  std::string build;

  Version() : major(1), minor(0), patch(0) {}

  inline Version(u32 _major, u32 _minor, u32 _patch = 0,
                 const std::string &_prerelease = "",
                 const std::string &_build = "")
      : major(_major), minor(_minor), patch(_patch), prerelease(_prerelease),
        build(_build) {}

  std::string to_string() const {
    std::string result = std::to_string(major) + "." + std::to_string(minor) +
                         "." + std::to_string(patch);
    if (!prerelease.empty()) {
      result += "-" + prerelease;
    }
    if (!build.empty()) {
      result += "+" + build;
    }
    return result;
  }

  static Version from_string(const std::string &version_str) {
    Version version;
    std::string str = version_str;

    if (str.empty())
      return Version(1, 0, 0);

    // Parse build metadata if present
    size_t plus_pos = str.find('+');
    if (plus_pos != std::string::npos) {
      version.build = str.substr(plus_pos + 1);
      str = str.substr(0, plus_pos);
    }

    // Parse prerelease if present
    size_t dash_pos = str.find('-');
    if (dash_pos != std::string::npos) {
      version.prerelease = str.substr(dash_pos + 1);
      str = str.substr(0, dash_pos);
    }

    // Parse major.minor.patch
    size_t dot1 = str.find('.');
    size_t dot2 = str.find('.', dot1 + 1);

    if (dot1 != std::string::npos) {
      version.major = std::stoi(str.substr(0, dot1));
      if (dot2 != std::string::npos) {
        version.minor = std::stoi(str.substr(dot1 + 1, dot2 - dot1 - 1));
        version.patch = std::stoi(str.substr(dot2 + 1));
      } else {
        version.minor = std::stoi(str.substr(dot1 + 1));
        version.patch = 0;
      }
    } else {
      version.major = std::stoi(str);
      version.minor = 0;
      version.patch = 0;
    }

    return version;
  }
};

enum class ProjectType { C, CPP };

class Project {
private:
  std::string name;
  Version version;
  std::string description;
  std::string author;
  std::string license;
  ProjectType type;
  std::string git_repo;
  std::string compiler_path;

  // New field: libraries
  std::vector<std::string> libs;

  std::string src_dir = "src";
  std::string lib_dir = "lib";
  std::string bin_dir = "bin";
  std::string inc_dir = "include";

  std::vector<std::string> dependencies;

  Filegen filegen;

public:
  inline Project(const std::string &arg0,
                 const std::string &project_data = "./project.json")
      : filegen(arg0) {
    if (fs::exists(project_data))
      parse(project_data);
  }

  inline Project(const std::string &arg0, const std::string &_name,
                 const Version &_version, const std::string &_description,
                 const std::string &_author, const std::string &_license,
                 ProjectType _type, const std::string &_git_repo,
                 const std::string &_compiler_path,
                 const std::vector<std::string> &_libs = {})
      : name(_name), version(_version), description(_description),
        author(_author), license(_license), type(_type), git_repo(_git_repo),
        compiler_path(_compiler_path), libs(_libs), filegen(arg0) {}

  static inline std::string
  prompt_with_default(const std::string &prompt,
                      const std::string &default_value,
                      bool non_interactive = false, bool newline = true) {
    if (non_interactive) {
      return default_value;
    }

    std::cout << prompt;
    if (!default_value.empty()) {
      std::cout << " (" << default_value << ") ";
    }

    std::cout << ": ";

    std::string input;
    std::getline(std::cin, input);

    if (input.empty() && !default_value.empty()) {
      return default_value;
    }
    return input;
  }

  static inline bool prompt_yes_no(const std::string &prompt,
                                   bool default_value,
                                   bool non_interactive = false) {
    if (non_interactive) {
      return default_value;
    }

    std::cout << prompt << " (y/n) [" << (default_value ? "Y" : "N") << "]: ";

    std::string input;
    std::getline(std::cin, input);

    if (input.empty()) {
      return default_value;
    }

    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
    return (input == "y" || input == "yes");
  }

  inline Project _init(bool non_interactive = false) {
    Project project(filegen.get_arg0());

    if (!non_interactive) {
      std::cout << "\n\x1b[36mThis utility will walk you through creating a "
                   "project.json file.\n";
      std::cout << "Press ^C at any time to quit.\x1b[0m\n\n";
    }

    // Get current directory name as default project name
    std::string default_name = "my-project";
    std::string current_dir = fs::current_path().filename().string();
    if (!current_dir.empty() && current_dir != ".") {
      default_name = current_dir;
    }

    // NPM-like prompts
    project.name =
        prompt_with_default("project name", default_name, non_interactive);
    project.version = Version::from_string(
        prompt_with_default("version", "1.0.0", non_interactive));
    project.description =
        prompt_with_default("description", "", non_interactive);
    project.author = prompt_with_default("author", "", non_interactive);
    project.license = prompt_with_default("license", "MIT", non_interactive);

    std::string type_str =
        prompt_with_default("project type (C/C++)", "C++", non_interactive);
    project.type = (type_str == "C" || type_str == "c") ? ProjectType::C
                                                        : ProjectType::CPP;

    project.git_repo =
        prompt_with_default("git repository", "", non_interactive);

    std::string default_compiler =
        (project.type == ProjectType::C) ? "clang" : "clang++";
    project.compiler_path =
        prompt_with_default("compiler path", default_compiler, non_interactive);

    // Prompt for directories
    project.src_dir = prompt_with_default("source_dir", "src", non_interactive);
    project.lib_dir =
        prompt_with_default("library_dir", "lib", non_interactive);
    project.bin_dir = prompt_with_default("binary_dir", "bin", non_interactive);
    project.inc_dir =
        prompt_with_default("include_dir", "include", non_interactive);

    // Ask about libraries
    if (!non_interactive) {
      std::cout << "\nEnter libraries (space-separated, press Enter to skip): ";
      std::string libs_input;
      std::getline(std::cin, libs_input);

      if (!libs_input.empty()) {
        size_t start = 0, end = 0;
        while ((end = libs_input.find(' ', start)) != std::string::npos) {
          if (end != start) {
            project.libs.push_back(libs_input.substr(start, end - start));
          }
          start = end + 1;
        }
        if (start < libs_input.length()) {
          project.libs.push_back(libs_input.substr(start));
        }
      }
    }

    // Show summary (skip in non-interactive mode)
    if (!non_interactive) {
      std::cout << "\n\x1b[32mAbout to write to " << fs::current_path().string()
                << "/project.json:\x1b[0m\n\n";

      json summary;
      summary["name"] = project.name;
      summary["version"] = project.version.to_string();
      summary["description"] = project.description;
      summary["author"] = project.author;
      summary["license"] = project.license;
      summary["type"] = (project.type == ProjectType::C) ? "C" : "C++";
      if (!project.git_repo.empty()) {
        summary["git_repo"] = project.git_repo;
      }
      summary["compiler_path"] = project.compiler_path;

      // Add directory fields
      summary["src_dir"] = project.src_dir;
      summary["lib_dir"] = project.lib_dir;
      summary["bin_dir"] = project.bin_dir;
      summary["inc_dir"] = project.inc_dir;

      if (!project.libs.empty()) {
        summary["libs"] = project.libs;
      }

      std::string confirm = prompt_with_default(
          "\x1b[33mIs this OK?\x1b[0m [Y/n]", "", false, false);

      std::transform(confirm.begin(), confirm.end(), confirm.begin(),
                     ::tolower);

      if (confirm == "yes" || confirm == "y" || confirm == "") {
        return project;
      } else {
        std::cout << "\x1b[31mAborted.\x1b[0m\n";
        exit(0);
      }
    } else {
      // Non-interactive mode - just create with defaults
      return project;
    }
  }

  inline void init(bool non_interactive = false) {
    *this = _init(non_interactive);

    std::system("git init");

    write();

    make_dirs();
    make_files();
  }

  inline void parse(const std::string &proj_file_path) {
    std::ifstream rstream(proj_file_path);
    AASSERT(rstream.good(), "Failed to read %s", proj_file_path.c_str());

    try {
      json read = json::parse(rstream);

      // Parse fields
      AASSERT(read.contains("name"),
              "Invalid project file %s, field name not found;",
              proj_file_path.c_str());
      name = read.value("name", "");

      if (read.contains("version")) {
        version = Version::from_string(read.value("version", "1.0.0"));
      }

      if (read.contains("description")) {
        description = read.value("description", "");
      }

      if (read.contains("author")) {
        author = read.value("author", "");
      }

      if (read.contains("license")) {
        license = read.value("license", "MIT");
      }

      if (read.contains("type")) {
        std::string type_str = read.value("type", "C++");
        type = (type_str == "C") ? ProjectType::C : ProjectType::CPP;
      }

      if (read.contains("git_repo")) {
        git_repo = read.value("git_repo", "");
      }

      if (read.contains("compiler_path")) {
        compiler_path = read.value("compiler_path", "");
      } else {
        compiler_path = (type == ProjectType::C) ? "clang" : "clang++";
      }

      // Parse directory fields
      if (read.contains("src_dir")) {
        src_dir = read.value("src_dir", "src");
      }
      if (read.contains("lib_dir")) {
        lib_dir = read.value("lib_dir", "lib");
      }
      if (read.contains("bin_dir")) {
        bin_dir = read.value("bin_dir", "bin");
      }
      if (read.contains("inc_dir")) {
        inc_dir = read.value("inc_dir", "include");
      }

      // Parse libs
      if (read.contains("libs")) {
        libs = read["libs"].get<std::vector<std::string>>();
      }

      if (read.contains("dependencies")) {
        dependencies = read["dependencies"].get<std::vector<std::string>>();
      }

    } catch (json::parse_error &e) {
      std::cerr << "Parse error: " << e.what() << std::endl;
    }
  }

  inline void write(const std::string &proj_file_path = "project.json") const {
    json data;

    // Write fields in npm-like order
    data["name"] = name;
    data["version"] = version.to_string();
    data["description"] = description;
    data["author"] = author;
    data["license"] = license;
    data["type"] = (type == ProjectType::C) ? "C" : "C++";
    if (!git_repo.empty()) {
      data["git_repo"] = git_repo;
    }
    data["compiler_path"] = compiler_path;

    // Add directory fields
    data["src_dir"] = src_dir;
    data["lib_dir"] = lib_dir;
    data["bin_dir"] = bin_dir;
    data["inc_dir"] = inc_dir;

    // Add file extension based on project type
    data["ext"] = (type == ProjectType::C) ? ".c" : ".cpp";

    if (!libs.empty()) {
      data["libs"] = libs;
    }

    if (!dependencies.empty()) {
      data["dependencies"] = dependencies;
    }

    std::ofstream write(proj_file_path);
    AASSERT(write.is_open(), "Failed to write to file %s;",
            proj_file_path.c_str());

    std::cout << std::setw(2) << data << std::endl;
    write << std::setw(2) << data << std::endl;

    std::cout << "\x1b[32m✓\x1b[0m Project file created: " << proj_file_path
              << std::endl;
  }

  inline void make_dirs() {
    fs::create_directories(src_dir);
    fs::create_directories(lib_dir);
    fs::create_directories(bin_dir);

    if (src_dir != inc_dir)
      fs::create_directories(inc_dir);
  }

  inline void make_files() {
    std::string ext = (type == ProjectType::C) ? ".c" : ".cpp";
    filegen.g_file("makefile");

    filegen.g_file(src_dir + "/main" + ext);
  }

  // Getters
  inline const std::string &get_name() const { return name; }
  inline const Version &get_version() const { return version; }
  inline const std::string &get_description() const { return description; }
  inline const std::string &get_author() const { return author; }
  inline const std::string &get_license() const { return license; }
  inline ProjectType get_type() const { return type; }
  inline const std::string &get_git_repo() const { return git_repo; }
  inline const std::string &get_compiler_path() const { return compiler_path; }
  inline const std::string &get_src_dir() const { return src_dir; }
  inline const std::string &get_lib_dir() const { return lib_dir; }
  inline const std::string &get_bin_dir() const { return bin_dir; }
  inline const std::string &get_inc_dir() const { return inc_dir; }
  inline const std::vector<std::string> &get_libs() const { return libs; }
  inline const std::vector<std::string> &get_dependencies() const {
    return dependencies;
  }
  inline std::string get_ext() const {
    return (type == ProjectType::C) ? ".c" : ".cpp";
  }

  // Setters
  inline void set_name(const std::string &value) { name = value; }
  inline void set_version(const Version &value) { version = value; }
  inline void set_description(const std::string &value) { description = value; }
  inline void set_author(const std::string &value) { author = value; }
  inline void set_license(const std::string &value) { license = value; }
  inline void set_type(ProjectType value) { type = value; }
  inline void set_git_repo(const std::string &value) { git_repo = value; }
  inline void set_compiler_path(const std::string &value) {
    compiler_path = value;
  }
  inline void set_src_dir(const std::string &value) { src_dir = value; }
  inline void set_lib_dir(const std::string &value) { lib_dir = value; }
  inline void set_bin_dir(const std::string &value) { bin_dir = value; }
  inline void set_inc_dir(const std::string &value) { inc_dir = value; }
  inline void set_libs(const std::vector<std::string> &value) { libs = value; }

  inline void add_lib(const std::string &lib) { libs.push_back(lib); }
  inline void add_dependency(const std::string &dep) {
    dependencies.push_back(dep);
  }
};
