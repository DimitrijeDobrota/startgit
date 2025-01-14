#pragma once

#include <string>
#include <vector>

#include <git2wrap/branch.hpp>

#include "commit.hpp"
#include "file.hpp"

namespace startgit
{

class repository;

class branch
{
public:
  explicit branch(git2wrap::branch brnch, repository& repo);
  branch(const branch&) = delete;
  branch& operator=(const branch&) = delete;
  branch(branch&&) = default;
  branch& operator=(branch&&) = default;
  ~branch() = default;

  const auto& get() const { return m_branch; }

  const std::string& get_name() const { return m_name; }
  const commit& get_last_commit() const { return m_commits[0]; }

  const auto& get_commits() const { return m_commits; }
  const auto& get_files() const { return m_files; }

private:
  git2wrap::branch m_branch;

  std::string m_name;

  std::vector<commit> m_commits;
  std::vector<file> m_files;
};

}  // namespace startgit
