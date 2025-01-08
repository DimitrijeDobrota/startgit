#pragma once

#include <string>

#include <git2wrap/branch.hpp>

namespace startgit
{

class branch
{
public:
  explicit branch(git2wrap::branch brnch);

  const std::string& get_name() const { return m_name; }

private:
  git2wrap::branch m_branch;

  std::string m_name;
};

}  // namespace startgit
