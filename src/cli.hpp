#pragma once

#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <map>
#include <print>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "utils.hpp"

template <typename T, typename A = std::allocator<T>>
using Arena = std::vector<T, A>;

// Enhanced Flag structure that can store values
// Update the Flag struct:
struct Flag {
  Arena<std::string> names;
  std::string description;
  bool is_set = false;
  bool expects_value = false;
  std::string value;
  int count = 0; // Add this to track how many times flag appears

  Flag(const Arena<std::string> &names, const std::string &description,
       bool expects_value = false)
      : names(names), description(description), expects_value(expects_value) {}
};

// Command structure for subcommands (formerly first_instruction)
struct Command {
  Arena<std::string> names;
  std::string description;

  Command(const Arena<std::string> &names, const std::string &description)
      : names(names), description(description) {}
};

class Cli {
private:
  Arena<std::string> args;
  Arena<Flag> flags;
  Arena<Command> commands;
  std::map<std::string, size_t> flag_map; // Store indices instead of pointers
  std::map<std::string, Command *> command_map; // For quick command lookup
  std::string active_command;      // The command that was found (if any)
  Arena<std::string> command_args; // Arguments after the command

  static constexpr const char *const fonts[] = {
      "banner", "block",    "digital",  "lean",     "mnemonic", "shadow",
      "small",  "smshadow", "standard", "big",      "bubble",   "ivrit",
      "mini",   "script",   "slant",    "smscript", "smslant",  "term"};

  static constexpr size_t fonts_count = sizeof(fonts) / sizeof(fonts[0]);

public:
  Cli(int ac, const char **argv) {
    if (ac > 0) {
      this->args = Arena<std::string>(argv + 1, argv + ac);
    }
  }

  Cli(const Arena<std::string> &arguments) : args(arguments) {}

  void add_flag(const Arena<std::string> &names, const std::string &description,
                bool expects_value = false) {
    flags.emplace_back(names, description, expects_value);
    size_t index = flags.size() - 1;
    for (const auto &name : names) {
      flag_map[name] = index;
    }
  }

  // Add a command (formerly first_instruction)
  void add_command(const Arena<std::string> &names,
                   const std::string &description) {
    commands.emplace_back(names, description);
    for (const auto &name : names) {
      command_map[name] = &commands.back();
    }
  }

  void args_break_down() {
    instructions.clear();
    active_command.clear();
    command_args.clear();

    // Reset flags
    for (auto &flag : flags) {
      flag.is_set = false;
      flag.value.clear();
      flag.count = 0; // Reset count
    }

    bool found_command = false;

    for (size_t i = 0; i < args.size(); ++i) {
      const auto &arg = args[i];

      if (!found_command && command_map.find(arg) != command_map.end()) {
        // Found a command
        active_command = arg;
        found_command = true;
      } else if (flag_map.find(arg) != flag_map.end()) {
        // Found a flag - use index to get the flag
        size_t index = flag_map[arg];
        Flag &flag = flags[index];
        flag.is_set = true;
        flag.count++; // Increment count

        if (flag.expects_value && i + 1 < args.size()) {
          // Next argument is the flag's value
          flag.value = args[++i];
        }
      } else {
        // It's either an instruction or a command argument
        if (found_command) {
          command_args.push_back(arg);
        } else {
          instructions.push_back(arg);
        }
      }
    }
  }

  /*
   * We are assuming this function isn't used
   * a lot so we will reinitialize the random_device
   * one every call instead of keeping it in the class
   */
  i64 get_rand(i64 floor, i64 ceiling) const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(floor, ceiling);
    return distrib(gen);
  }

  // Enhanced help printing with commands
  void print_help() const {
    std::cout << ANSI_BOLD << ANSI_FG_YELLOW << '\n';
    std::system(std::string(std::string("echo cooki | figlet -f ") +
                            fonts[get_rand(0, fonts_count - 1)])
                    .c_str());

    std::cout << ANSI_RESET << "Usage:\n";
    if (!commands.empty()) {
      std::cout << "cooki [FLAGS] <COMMAND> [ARGS...]\n\n";
    } else {
      std::cout << "cooki [FLAGS] [INSTRUCTIONS...]\n\n";
    }

    if (!commands.empty()) {
      std::cout << "Commands:\n";
      for (const auto &cmd : commands) {
        std::cout << "  ";
        bool first = true;
        for (const auto &name : cmd.names) {
          if (!first)
            std::cout << ", ";
          std::cout << name;
          first = false;
        }
        std::cout << "\n      " << cmd.description << "\n";
      }
      std::cout << "\n";
    }

    if (!flags.empty()) {
      std::cout << "Flags:\n";
      for (const auto &flag : flags) {
        std::cout << "  ";
        bool first = true;
        for (const auto &name : flag.names) {
          if (!first)
            std::cout << ", ";
          std::cout << name;
          first = false;
        }
        if (flag.expects_value) {
          std::cout << " <value>";
        }
        std::cout << "\n      " << flag.description << "\n";
      }
    }

    if (!instructions.empty() && commands.empty()) {
      std::cout << "\nInstructions:\n";
      for (const auto &instr : instructions) {
        std::cout << "  " << instr << "\n";
      }
    }
  }

  // Get flag value if flag exists and was set with a value
  std::string get_flag_value(const std::string &flag_name) const {
    auto it = flag_map.find(flag_name);
    if (it != flag_map.end() && flags[it->second].is_set) {
      return flags[it->second].value;
    }
    return "";
  }

  // Check if flag exists and was set
  bool is_flag_set(const std::string &flag_name) const {
    auto it = flag_map.find(flag_name);
    if (it != flag_map.end()) {
      return flags[it->second].is_set;
    }
    return false;
  }

  // Add this method to the Cli class:
  std::vector<std::string> get_set_flags() const {
    std::vector<std::string> result;
    for (const auto &pair : flag_map) {
      if (flags[pair.second].is_set) {
        result.push_back(pair.first);
      }
    }
    return result;
  }

  // And update warn_unused_flags:
  // In cli.hpp, update the warn_unused_flags method:
  void warn_unused_flags(const std::set<std::string> &valid_flags) const {
    std::set<size_t>
        already_warned; // Track which flag indices we've already warned about

    for (const auto &flag_name : get_set_flags()) {
      auto it = flag_map.find(flag_name);
      if (it == flag_map.end())
        continue;

      size_t flag_index = it->second;

      // Skip if we've already warned about this flag (handles aliases)
      if (already_warned.find(flag_index) != already_warned.end()) {
        continue;
      }

      const Flag &flag = flags[flag_index];

      // Check if any of this flag's names are in valid_flags
      bool is_valid = false;
      for (const auto &name : flag.names) {
        if (valid_flags.find(name) != valid_flags.end()) {
          is_valid = true;
          break;
        }
      }

      // If flag is set but not valid (and not help flag), warn about it
      if (flag.is_set && !is_valid) {
        // Skip help flags
        bool is_help_flag = false;
        for (const auto &name : flag.names) {
          if (name == "-h" || name == "--help") {
            is_help_flag = true;
            break;
          }
        }

        if (!is_help_flag) {
          // Format all names for this flag
          std::string all_names;
          for (size_t i = 0; i < flag.names.size(); ++i) {
            if (i > 0)
              all_names += ", ";
            all_names += flag.names[i];
          }
          std::cout << WARN_MSG << "unused flag {" << all_names << "}\n";
          already_warned.insert(flag_index);
        }
      }
    }
  }

  // Add this method to the Cli class:
  void check_duplicate_flags() const {
    for (const auto &flag : flags) {
      if (flag.count > 1) {
        std::string all_names;
        for (size_t i = 0; i < flag.names.size(); ++i) {
          if (i > 0)
            all_names += ", ";
          all_names += flag.names[i];
        }

        std::cout << WARN_MSG << "flag {" << all_names << "} specified "
                  << flag.count << " times\n";
      }
    }
  }

  // Check if a command was used
  bool has_command() const { return !active_command.empty(); }

  // Get the active command name
  std::string get_command() const { return active_command; }

  // Get arguments for the active command
  const Arena<std::string> &get_command_args() const { return command_args; }

  Arena<Flag> get_flags() const { return this->flags; }

  const Arena<std::string> &get_instructions() const { return instructions; }

private:
  // Kept for backward compatibility - contains instructions when no command is
  // used
  Arena<std::string> instructions;

  // Example usage in main:
  /*
  int main(int argc, char** argv) {
      Cli cli(argc, argv);

      cli.add_flag({"-h", "--help"}, "Show help message");
      cli.add_flag({"-v", "--verbose"}, "Enable verbose output");
      cli.add_flag({"-o", "--output"}, "Output file name", true);

      cli.add_command({"init", "i"}, "Initialize a new project");
      cli.add_command({"build", "b"}, "Build the project");
      cli.add_command({"run", "r"}, "Run the project");

      cli.args_break_down();

      if (cli.is_flag_set("-h") || cli.is_flag_set("--help")) {
          cli.print_help();
          return 0;
      }

      if (cli.has_command()) {
          std::string cmd = cli.get_command();
          auto cmd_args = cli.get_command_args();

          if (cmd == "init") {
              // Handle init command
          } else if (cmd == "build") {
              // Handle build command
          } else if (cmd == "run") {
              // Handle run command
          }
      } else {
          auto instructions = cli.get_instructions();
          std::cout << "Instructions:\n";
          for (const auto& instr : instructions) {
              std::cout << "  " << instr << "\n";
          }
      }

      return 0;
  }
  */
};
