#pragma once

#include "cli.hpp"

class Gitman {
public:
};

class Pacman {
public:
  void install(const std::string &package_name) {}

  void install(const Arena<std::string> &package_names) {
    for (const auto &package_name : package_names)
      this->install(package_name);
  }
};
