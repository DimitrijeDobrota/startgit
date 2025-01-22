#pragma once

#include <filesystem>
#include <string>
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
  bool force = false;
};

extern arguments_t args;  // NOLINT

}  // namespace startgit
