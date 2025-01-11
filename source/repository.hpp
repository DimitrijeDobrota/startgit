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

  const auto& get() const { return m_repo; }
  const auto& get_path() const { return m_path; }

  const std::string& get_url() const { return m_url; }
  const std::string& get_name() const { return m_name; }
  const std::string& get_owner() const { return m_owner; }
  const std::string& get_description() const { return m_description; }

  const auto& get_branches() const { return m_branches; }

private:
  static std::string read_file(const std::filesystem::path& base,
                               const char* file);

  std::filesystem::path m_path;
  git2wrap::repository m_repo;

  std::string m_name;

  std::string m_url;
  std::string m_owner;
  std::string m_description;

  std::vector<branch> m_branches;
};

}  // namespace startgit
