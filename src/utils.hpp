#pragma once

#include <array>
#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <vector>

// ============================================================
// INTEGER TYPE ALIASES
// ============================================================
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

// ============================================================
// FLOATING-POINT TYPE ALIASES
// ============================================================
using f32 = float;
using f64 = double;

namespace fs = std::filesystem;

// ============================================================
// ANSI SGR CODES (All as constexpr std::string_view for efficiency)
// ============================================================

// Attributes
constexpr std::string_view ANSI_RESET = "\033[0m";
constexpr std::string_view ANSI_BOLD = "\033[1m";
constexpr std::string_view ANSI_DIM = "\033[2m";
constexpr std::string_view ANSI_UNDERLINE = "\033[4m";
constexpr std::string_view ANSI_BLINK = "\033[5m";
constexpr std::string_view ANSI_REVERSE = "\033[7m";
constexpr std::string_view ANSI_HIDDEN = "\033[8m";

// Standard foreground colors (30–37)
constexpr std::string_view ANSI_FG_BLACK = "\033[30m";
constexpr std::string_view ANSI_FG_RED = "\033[31m";
constexpr std::string_view ANSI_FG_GREEN = "\033[32m";
constexpr std::string_view ANSI_FG_YELLOW = "\033[33m";
constexpr std::string_view ANSI_FG_BLUE = "\033[34m";
constexpr std::string_view ANSI_FG_MAGENTA = "\033[35m";
constexpr std::string_view ANSI_FG_CYAN = "\033[36m";
constexpr std::string_view ANSI_FG_WHITE = "\033[37m";

// Bright foreground colors (90–97)
constexpr std::string_view ANSI_FG_BRIGHT_BLACK = "\033[90m";
constexpr std::string_view ANSI_FG_BRIGHT_RED = "\033[91m";
constexpr std::string_view ANSI_FG_BRIGHT_GREEN = "\033[92m";
constexpr std::string_view ANSI_FG_BRIGHT_YELLOW = "\033[93m";
constexpr std::string_view ANSI_FG_BRIGHT_BLUE = "\033[94m";
constexpr std::string_view ANSI_FG_BRIGHT_MAGENTA = "\033[95m";
constexpr std::string_view ANSI_FG_BRIGHT_CYAN = "\033[96m";
constexpr std::string_view ANSI_FG_BRIGHT_WHITE = "\033[97m";

// Standard background colors (40–47)
constexpr std::string_view ANSI_BG_BLACK = "\033[40m";
constexpr std::string_view ANSI_BG_RED = "\033[41m";
constexpr std::string_view ANSI_BG_GREEN = "\033[42m";
constexpr std::string_view ANSI_BG_YELLOW = "\033[43m";
constexpr std::string_view ANSI_BG_BLUE = "\033[44m";
constexpr std::string_view ANSI_BG_MAGENTA = "\033[45m";
constexpr std::string_view ANSI_BG_CYAN = "\033[46m";
constexpr std::string_view ANSI_BG_WHITE = "\033[47m";

// Bright background colors (100–107)
constexpr std::string_view ANSI_BG_BRIGHT_BLACK = "\033[100m";
constexpr std::string_view ANSI_BG_BRIGHT_RED = "\033[101m";
constexpr std::string_view ANSI_BG_BRIGHT_GREEN = "\033[102m";
constexpr std::string_view ANSI_BG_BRIGHT_YELLOW = "\033[103m";
constexpr std::string_view ANSI_BG_BRIGHT_BLUE = "\033[104m";
constexpr std::string_view ANSI_BG_BRIGHT_MAGENTA = "\033[105m";
constexpr std::string_view ANSI_BG_BRIGHT_CYAN = "\033[106m";
constexpr std::string_view ANSI_BG_BRIGHT_WHITE = "\033[107m";

// ============================================================
// FORMATTED MESSAGES
// ============================================================
inline const std::string PROJECT_MSG = std::string(ANSI_FG_YELLOW) +
                                       std::string(ANSI_BOLD) +
                                       "Cooki: " + std::string(ANSI_RESET);

inline const std::string ERROR_MSG = PROJECT_MSG + std::string(ANSI_BOLD) +
                                     std::string(ANSI_FG_RED) +
                                     "error: " + std::string(ANSI_RESET);

inline const std::string WARN_MSG = PROJECT_MSG + std::string(ANSI_BOLD) +
                                    std::string(ANSI_FG_MAGENTA) +
                                    "warning: " + std::string(ANSI_RESET);

// ============================================================
// ASSERTION MACRO
// ============================================================
#define AASSERT(expr, msg, ...)                                                \
  do {                                                                         \
    if (!(expr)) {                                                             \
      std::fprintf(stderr, "%s", ERROR_MSG.c_str());                           \
      std::fprintf(stderr, msg, ##__VA_ARGS__);                                \
      std::fprintf(stderr, "\n%s%sAborted%s\n", PROJECT_MSG.c_str(),           \
                   ANSI_BOLD.data(), ANSI_RESET.data());                       \
      std::abort();                                                            \
    }                                                                          \
  } while (0)

// ============================================================
// SYSTEM COMMAND EXECUTION
// ============================================================
inline std::string run_cmd(const std::string &cmd) {
  AASSERT(!cmd.empty(), "Command string cannot be empty");

  std::array<char, 256> buffer{};
  std::string result;

  FILE *pipe = popen(cmd.c_str(), "r");
  AASSERT(pipe != nullptr, "Failed to open pipe for command: %s", cmd.c_str());

  while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) !=
         nullptr) {
    result += buffer.data();
  }

  int status = pclose(pipe);
  AASSERT(status == 0, "Command failed with exit code %d: %s", status,
          cmd.c_str());

  // Remove trailing newline if present
  if (!result.empty() && result.back() == '\n') {
    result.pop_back();
  }

  return result;
}

// ============================================================
// EXECUTABLE PATH FINDING
// ============================================================
inline std::string find_exec_path(const std::string &cmd) {
  AASSERT(!cmd.empty(), "Command name cannot be empty");
  return run_cmd("command -v " + cmd);
}

// ============================================================
// DEFAULT COMPILER DETECTION
// ============================================================
inline std::string find_default_compiler() {
  // Prefer clang++
  if (auto path = find_exec_path("clang++"); !path.empty()) {
    return path;
  }

  // Fallback to g++
  if (auto path = find_exec_path("g++"); !path.empty()) {
    return path;
  }

  // Last resort: generic cc with C++ flag
  if (auto path = find_exec_path("cc"); !path.empty()) {
    return path + " -x c++";
  }

  AASSERT(false, "No C++ compiler found (tried: clang++, g++, cc)");
  return ""; // Unreachable due to assertion
}

// ============================================================
// FILE SYSTEM UTILITIES
// ============================================================
inline bool file_exists(const std::string &file_path) {
  AASSERT(!file_path.empty(), "File path cannot be empty");
  return fs::exists(file_path);
}

inline std::string readfile(const fs::path &path) {
  AASSERT(fs::exists(path), "File does not exist: %s", path.string().c_str());
  AASSERT(!fs::is_directory(path), "Path is a directory, not a file: %s",
          path.string().c_str());

  std::ifstream in(path, std::ios::binary | std::ios::ate);
  AASSERT(in.is_open(), "Failed to open file for reading: %s",
          path.string().c_str());

  const auto size = in.tellg();
  AASSERT(size >= 0, "Failed to determine file size: %s",
          path.string().c_str());

  std::string contents(static_cast<std::size_t>(size), '\0');
  in.seekg(0);

  const auto bytes_read = in.read(&contents[0], size).gcount();
  AASSERT(bytes_read == size,
          "Read mismatch: expected %lld bytes, got %lld bytes",
          static_cast<long long>(size), static_cast<long long>(bytes_read));

  return contents;
}

// Overload for std::string path
inline std::string readfile(const std::string &path) {
  AASSERT(!path.empty(), "File path cannot be empty");
  return readfile(fs::path(path));
}

inline bool writefile(const fs::path &path, const std::string &content) {
  AASSERT(!path.empty(), "Output path cannot be empty");

  // Ensure parent directory exists
  const auto parent_dir = path.parent_path();
  if (!parent_dir.empty() && !fs::exists(parent_dir)) {
    AASSERT(fs::create_directories(parent_dir),
            "Failed to create parent directory: %s",
            parent_dir.string().c_str());
  }

  std::ofstream file(path, std::ios::binary);
  AASSERT(file.is_open(), "Failed to open file for writing: %s",
          path.string().c_str());

  file << content;
  AASSERT(file.good(), "Failed to write content to file: %s",
          path.string().c_str());

  return true;
}

// Overload for std::string path
inline bool writefile(const std::string &path, const std::string &content) {
  AASSERT(!path.empty(), "Output path cannot be empty");
  return writefile(fs::path(path), content);
}

// ============================================================
// ADDITIONAL HELPER FUNCTIONS
// ============================================================
inline std::size_t file_size(const fs::path &path) {
  AASSERT(fs::exists(path), "File does not exist: %s", path.string().c_str());
  AASSERT(!fs::is_directory(path), "Path is a directory, not a file: %s",
          path.string().c_str());

  const auto size = fs::file_size(path);
  AASSERT(size != static_cast<std::uintmax_t>(-1),
          "Failed to get file size: %s", path.string().c_str());

  return static_cast<std::size_t>(size);
}

// String manipulation utilities
inline std::string trim(const std::string &str) {
  auto front = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
    return std::isspace(ch);
  });
  auto back = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
                return std::isspace(ch);
              }).base();
  return (front < back) ? std::string(front, back) : std::string();
}

inline std::vector<std::string> split(const std::string &str, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream token_stream(str);

  while (std::getline(token_stream, token, delimiter)) {
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }

  return tokens;
}

inline bool ends_with(const std::string &str, const std::string &suffix) {
  return str.size() >= suffix.size() &&
         str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

inline bool starts_with(const std::string &str, const std::string &prefix) {
  return str.size() >= prefix.size() &&
         str.compare(0, prefix.size(), prefix) == 0;
}
