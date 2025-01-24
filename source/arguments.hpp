#pragma once

#include <filesystem>
#include <string>
#include <unordered_set>
#include <vector>

namespace startgit
{

struct arguments_t
{
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
