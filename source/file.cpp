#include "file.hpp"

#include "utils.hpp"

namespace startgit
{

file::file(const git2wrap::tree_entry& entry, std::string path)
    : m_filemode(filemode(entry.get_filemode()))
    , m_path(std::move(path))
{
}

}  // namespace startgit
