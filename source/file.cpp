#include <algorithm>
#include <span>

#include "file.hpp"

#include <git2wrap/repository.hpp>

#include "utils.hpp"

namespace startgit
{

file::file(const git2wrap::tree_entry& entry, std::filesystem::path path)
    : m_filemode(filemode(entry.get_filemode()))
    , m_path(std::move(path))
    , m_blob(git2wrap::repository(entry.get_owner()).blob_lookup(entry.get_id())
      )
{
}

bool file::is_binary() const
{
  return m_blob.is_binary();
}

const char* file::get_content() const
{
  return static_cast<const char*>(m_blob.get_rawcontent());
}

git2wrap::object_size_t file::get_size() const
{
  return m_blob.get_rawsize();
}

int file::get_lines() const
{
  if (m_lines != -1) {
    return m_lines;
  }

  const auto span = std::span<const char>(get_content(), get_size());
  return m_lines = static_cast<int>(std::count(span.begin(), span.end(), '\n'));
}

}  // namespace startgit
