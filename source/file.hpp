#pragma once

#include <string>

#include <git2wrap/tree.hpp>

namespace startgit
{

class file
{
public:
  file(const git2wrap::tree_entry& entry, std::string path);

  std::string get_filemode() const { return m_filemode; }
  std::string get_path() const { return m_path; }

private:
  std::string m_filemode;
  std::string m_path;
};

}  // namespace startgit
