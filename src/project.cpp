#include "project.hpp"

// Version implementation
Version::Version() : major(1), minor(0), patch(0) {}

Version::Version(u32 _major, u32 _minor, u32 _patch,
                 const std::string &_prerelease, const std::string &_build)
    : major(_major), minor(_minor), patch(_patch), prerelease(_prerelease),
      build(_build) {}

std::string Version::to_string() const {
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

Version Version::from_string(const std::string &version_str) {
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

// Project implementation
Project::Project(const std::string &arg0, const std::string &project_data)
    : arg0(arg0), src_dir("src"), lib_dir("lib"), bin_dir("bin"),
      inc_dir("include"), create_test(true), api_prefix("") {
  if (fs::exists(project_data))
    parse(project_data);
}

Project::Project(const std::string &arg0, const std::string &_name,
                 const Version &_version, const std::string &_description,
                 const std::string &_author, const std::string &_license,
                 ProjectLang _lang, const std::string &_git_repo,
                 const std::string &_compiler_path,
                 const std::vector<std::string> &_libs)
    : name(_name), version(_version), description(_description),
      author(_author), license(_license), lang(_lang), git_repo(_git_repo),
      compiler_path(_compiler_path), libs(_libs), src_dir("src"),
      lib_dir("lib"), bin_dir("bin"), inc_dir("include"), create_test(true),
      api_prefix(""), arg0(arg0) {}

void Project::setup_filegen() {
  if (type == ProjectType::EXE) {
    filegen = std::make_unique<ExeFilegen>(arg0);
  } else {
    filegen = std::make_unique<LibFilegen>(arg0);
  }
}

void Project::set_type(ProjectType value) {
  type = value;
  setup_filegen();
}

std::string Project::prompt_with_default(const std::string &prompt,
                                         const std::string &default_value,
                                         bool non_interactive, bool newline) {
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

bool Project::prompt_yes_no(const std::string &prompt, bool default_value,
                            bool non_interactive) {
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

Project Project::_init(bool non_interactive) {
  Project project(arg0);

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

  // Project type
  std::string type_str =
      prompt_with_default("project type", "Executable", non_interactive);
  std::string lower_type = type_str;
  std::transform(lower_type.begin(), lower_type.end(), lower_type.begin(),
                 ::tolower);
  project.type = (lower_type == "executable" || lower_type == "exe")
                     ? ProjectType::EXE
                     : ProjectType::LIB;

  // Setup filegen based on type
  project.setup_filegen();

  // Common prompts
  project.version = Version::from_string(
      prompt_with_default("version", "1.0.0", non_interactive));
  project.description = prompt_with_default("description", "", non_interactive);
  project.author = prompt_with_default("author", "", non_interactive);
  project.license = prompt_with_default("license", "MIT", non_interactive);

  std::string lang_str =
      prompt_with_default("project lang (C/C++)", "C++", non_interactive);
  project.lang =
      (lang_str == "C" || lang_str == "c") ? ProjectLang::C : ProjectLang::CPP;

  project.git_repo = prompt_with_default("git repository", "", non_interactive);

  std::string default_compiler =
      (project.lang == ProjectLang::C) ? "clang" : "clang++";
  project.compiler_path =
      prompt_with_default("compiler path", default_compiler, non_interactive);

  // Prompt for directories
  project.src_dir = prompt_with_default("source_dir", "src", non_interactive);
  project.lib_dir = prompt_with_default("library_dir", "lib", non_interactive);
  project.bin_dir = prompt_with_default("binary_dir", "bin", non_interactive);
  project.inc_dir =
      prompt_with_default("include_dir", "include", non_interactive);

  // Library-specific prompts
  if (project.type == ProjectType::LIB) {
    std::cout << "\n\x1b[33mLibrary-specific configuration:\x1b[0m\n";

    // API prefix
    project.api_prefix = prompt_with_default("API function prefix",
                                             project.name, non_interactive);

    // Test file
    project.create_test =
        prompt_yes_no("Create test file?", true, non_interactive);
  }

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
    summary["type"] =
        (project.type == ProjectType::EXE) ? "Executable" : "Library";
    summary["version"] = project.version.to_string();
    summary["description"] = project.description;
    summary["author"] = project.author;
    summary["license"] = project.license;
    summary["lang"] = (project.lang == ProjectLang::C) ? "C" : "C++";
    if (!project.git_repo.empty()) {
      summary["git_repo"] = project.git_repo;
    }
    summary["compiler_path"] = project.compiler_path;

    // Add directory fields
    summary["src_dir"] = project.src_dir;
    summary["lib_dir"] = project.lib_dir;
    summary["bin_dir"] = project.bin_dir;
    summary["inc_dir"] = project.inc_dir;

    // Add library-specific fields
    if (project.type == ProjectType::LIB) {
      summary["create_test"] = project.create_test;
      summary["api_prefix"] = project.api_prefix;
    }

    if (!project.libs.empty()) {
      summary["libs"] = project.libs;
    }

    std::string confirm = prompt_with_default(
        "\x1b[33mIs this OK?\x1b[0m [Y/n]", "", false, false);

    std::transform(confirm.begin(), confirm.end(), confirm.begin(), ::tolower);

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

void Project::init(bool non_interactive) {
  *this = _init(non_interactive);

  std::system("git init");

  write();

  make_dirs();
  make_files();
}

void Project::parse(const std::string &proj_file_path) {
  std::ifstream rstream(proj_file_path);
  AASSERT(rstream.good(), "Failed to read %s", proj_file_path.c_str());

  try {
    json read = json::parse(rstream);

    // Parse fields
    AASSERT(read.contains("name"),
            "Invalid project file %s, field name not found;",
            proj_file_path.c_str());
    name = read.value("name", "");

    // Parse type field
    if (read.contains("type")) {
      std::string type_str = read.value("type", "Executable");
      std::string lower_type = type_str;
      std::transform(lower_type.begin(), lower_type.end(), lower_type.begin(),
                     ::tolower);
      type = (lower_type == "executable" || lower_type == "exe")
                 ? ProjectType::EXE
                 : ProjectType::LIB;
    }

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

    if (read.contains("lang")) {
      std::string lang_str = read.value("lang", "C++");
      lang = (lang_str == "C") ? ProjectLang::C : ProjectLang::CPP;
    }

    if (read.contains("git_repo")) {
      git_repo = read.value("git_repo", "");
    }

    if (read.contains("compiler_path")) {
      compiler_path = read.value("compiler_path", "");
    } else {
      compiler_path = (lang == ProjectLang::C) ? "clang" : "clang++";
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

    // Parse library-specific fields
    if (read.contains("create_test")) {
      create_test = read.value("create_test", true);
    }
    if (read.contains("api_prefix")) {
      api_prefix = read.value("api_prefix", name);
    }

    // Parse libs
    if (read.contains("libs")) {
      libs = read["libs"].get<std::vector<std::string>>();
    }

    if (read.contains("dependencies")) {
      dependencies = read["dependencies"].get<std::vector<std::string>>();
    }

    // Setup filegen after parsing
    setup_filegen();

  } catch (json::parse_error &e) {
    std::cerr << "Parse error: " << e.what() << std::endl;
  }
}

void Project::write(const std::string &proj_file_path) const {
  json data;

  // Write fields in npm-like order
  data["name"] = name;
  data["type"] = (type == ProjectType::EXE) ? "Executable" : "Library";
  data["version"] = version.to_string();
  data["description"] = description;
  data["author"] = author;
  data["license"] = license;
  data["lang"] = (lang == ProjectLang::C) ? "C" : "C++";
  if (!git_repo.empty()) {
    data["git_repo"] = git_repo;
  }
  data["compiler_path"] = compiler_path;

  // Add directory fields
  data["src_dir"] = src_dir;
  data["lib_dir"] = lib_dir;
  data["bin_dir"] = bin_dir;
  data["inc_dir"] = inc_dir;

  // Add file extension based on project lang
  data["ext"] = (lang == ProjectLang::C) ? ".c" : ".cpp";

  // Add library-specific fields
  if (type == ProjectType::LIB) {
    data["create_test"] = create_test;
    data["api_prefix"] = api_prefix;
  }

  if (!libs.empty()) {
    data["libs"] = libs;
  }

  if (!dependencies.empty()) {
    data["dependencies"] = dependencies;
  }

  std::ofstream write_stream(proj_file_path);
  AASSERT(write_stream.is_open(), "Failed to write to file %s;",
          proj_file_path.c_str());

  std::cout << std::setw(2) << data << std::endl;
  write_stream << std::setw(2) << data << std::endl;

  std::cout << "\x1b[32m✓\x1b[0m Project file created: " << proj_file_path
            << std::endl;
}

void Project::make_exe_dirs() {
  fs::create_directories(src_dir);
  fs::create_directories(lib_dir);
  fs::create_directories(bin_dir);

  if (src_dir != inc_dir)
    fs::create_directories(inc_dir);
}

void Project::make_lib_dirs() {
  fs::create_directories(src_dir);
  fs::create_directories(lib_dir);
  fs::create_directories(bin_dir);

  // Create include directory with project subdirectory
  std::string project_inc_dir = inc_dir + "/" + name;
  fs::create_directories(project_inc_dir);
}

void Project::make_dirs() {
  if (type == ProjectType::EXE) {
    make_exe_dirs();
  } else {
    make_lib_dirs();
  }
}

void Project::make_exe_files() {
  if (!filegen) {
    setup_filegen();
  }

  ExeFilegen *exe_gen = dynamic_cast<ExeFilegen *>(filegen.get());
  if (exe_gen) {
    exe_gen->generate_makefile(name);
    exe_gen->generate_main_file(src_dir, name, lang == ProjectLang::CPP);
  }
}

void Project::make_lib_files() {
  if (!filegen) {
    setup_filegen();
  }

  LibFilegen *lib_gen = dynamic_cast<LibFilegen *>(filegen.get());
  if (lib_gen) {
    lib_gen->generate_makefile(name);
    lib_gen->generate_header_file(inc_dir, name, lang == ProjectLang::CPP,
                                  description, author);
    lib_gen->generate_source_file(src_dir, name, lang == ProjectLang::CPP);

    if (create_test) {
      // hmmmm
      // lib_gen->generate_test_file(src_dir, name, lang == ProjectLang::CPP);
    }
  }
}

void Project::make_files() {
  if (type == ProjectType::EXE) {
    make_exe_files();
  } else {
    make_lib_files();
  }
}

std::string Project::get_ext() const {
  return (lang == ProjectLang::C) ? ".c" : ".cpp";
}
