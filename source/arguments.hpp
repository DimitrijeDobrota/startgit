#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace startgit
{

struct arguments_t
{
  void add_special(std::string_view value) { special.insert(value); }

  void set_repository(std::string_view value)
  {
    repos.emplace_back(std::filesystem::canonical(value));
  }

  void set_resource(std::string_view value)
  {
    resource_url = value;
    if (resource_url.back() == '/') {
      resource_url.pop_back();
    }
  }

  void set_base(std::string_view value)
  {
    base_url = value;
    if (base_url.back() == '/') {
      base_url.pop_back();
    }
  }

  std::filesystem::path output_dir = ".";
  std::vector<std::filesystem::path> repos;
  std::string resource_url = "https://dimitrijedobrota.com";
  std::string base_url = "https://git.dimitrijedobrota.com";
  std::string author = "Dimitrije Dobrota";
  std::string title = "Collection of git repositories";
  std::string description = "Publicly available personal projects";
  std::string github = "DimitrijeDobrota";
  std::unordered_set<std::filesystem::path> special = {
      "BUILDING.md",
      "CODE_OF_CONDUCT.md",
      "CONTRIBUTING.md",
      "HACKING.md",
      "LICENSE.md",
      "README.md",
  };
  bool force = false;
};

extern arguments_t args;  // NOLINT

}  // namespace startgit
