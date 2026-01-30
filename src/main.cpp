#include <iostream>
#include <print>
#include <set>

#include "cli.hpp"
#include "project.hpp"

int main(int ac, const char **argv) {
  Cli cli(ac, argv);
  Project project(argv[0]);

  cli.add_flag({"-h", "--help"}, "Show help");
  cli.add_flag({"-y", "--yes"}, "Use defaults when initing project");
  cli.add_flag({"-g", "--global"}, "Install globally");

  cli.add_command({"install", "i"}, "Installs a library");
  cli.add_command({"init"}, "Initializes the project");

  cli.args_break_down();

  // Check for duplicate flags FIRST
  cli.check_duplicate_flags();

  if (cli.is_flag_set("-h") || cli.is_flag_set("--help")) {
    cli.print_help();
    return 0;
  }

  // Check which command was used
  if (cli.has_command()) {
    std::string command = cli.get_command();
    const auto &cmd_args = cli.get_command_args();

    if (command == "init") {
      // Define which flags are valid for init
      std::set<std::string> valid_flags_for_init = {"-y", "--yes"};

      // Check for unused flags
      cli.warn_unused_flags(valid_flags_for_init);

      // Handle init command
      bool use_defaults = cli.is_flag_set("-y") || cli.is_flag_set("--yes");
      project.init(use_defaults);

      // Check for unused command arguments
      for (const auto &arg : cmd_args) {
        std::cout << WARN_MSG << "argument '" << arg
                  << "' is not used with 'init' command\n";
      }

    } else if (command == "install" || command == "i") {
      // Define which flags are valid for install
      std::set<std::string> valid_flags_for_install = {"-g", "--global"};

      // Check for unused flags
      cli.warn_unused_flags(valid_flags_for_install);

      // Handle install command
      bool global = cli.is_flag_set("-g") || cli.is_flag_set("--global");

      // cmd_args contains what to install
      if (cmd_args.empty()) {
        std::println("No libraries specified to install");
      } else {
        for (const auto &library : cmd_args) {
          std::println("Installing {} (global: {})", library, global);
        }
      }
    }
  } else {
    // No command specified - no flags are valid except -h
    std::set<std::string> valid_flags_no_command = {"-h", "--help"};
    cli.warn_unused_flags(valid_flags_no_command);

    const auto &instructions = cli.get_instructions();

    if (!instructions.empty()) {
      std::cout << WARN_MSG
                << "arguments were provided but no command was specified\n";
      std::println("Available commands: init, install");
    } else {
      // No command and no instructions - show help
      cli.print_help();
    }
  }

  return 0;
}
