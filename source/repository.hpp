#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <git2wrap/repository.hpp>

#include "branch.hpp"

namespace startgit
{

class repository
{
public:
  explicit repository(const std::filesystem::path& path);

  const git2wrap::repository& get() const { return m_repo; }

  const std::string& get_name() const { return m_name; }
  const std::string& get_owner() const { return m_owner; }
  const std::string& get_description() const { return m_description; }

  const auto& get_branches() const { return m_branches; }

private:
  static void read_file(std::string& out,
                        const std::filesystem::path& base,
                        const char* file);

  git2wrap::repository m_repo;

  std::string m_name;
  std::string m_owner = "Unknown";
  std::string m_description;

  std::vector<branch> m_branches;
};

}  // namespace startgit
