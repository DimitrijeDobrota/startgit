#include <fstream>

#include "repository.hpp"

namespace startgit
{

repository::repository(const std::filesystem::path& path)
    : m_path(path)
    , m_repo(git2wrap::repository::open(
          path.c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, nullptr))
    , m_name(path.stem().string())
    , m_url(read_file(path, "url"))
    , m_owner(read_file(path, "owner"))
    , m_description(read_file(path, "description"))
{
  for (auto it = m_repo.branch_begin(GIT_BRANCH_LOCAL);
       it != m_repo.branch_end();
       ++it)
  {
    m_branches.emplace_back(it->dup());
  }
}

std::string repository::read_file(const std::filesystem::path& base,
                                  const char* file)
{
  std::ifstream ifs(base / file);

  if (!ifs.is_open()) {
    ifs = base / ".git" / file;
  }

  if (ifs.is_open()) {
    std::string res;
    std::getline(ifs, res, '\n');
    return res;
  }

  return "Unknown";
}

}  // namespace startgit
