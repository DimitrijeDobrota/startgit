#pragma once

#include <filesystem>
#include <string>

#include <git2wrap/blob.hpp>
#include <git2wrap/tree.hpp>

namespace startgit
{

class file
{
public:
  file(const git2wrap::tree_entry& entry, std::string path);

  std::string get_filemode() const { return m_filemode; }
  std::filesystem::path get_path() const { return m_path; }

  bool is_binary() const;
  const char* get_content() const;
  git2wrap::object_size_t get_size() const;

private:
  std::string m_filemode;
  std::filesystem::path m_path;
  git2wrap::blob m_blob;
};

}  // namespace startgit
