#include <fstream>

#include "repository.hpp"

namespace startgit
{

repository::repository(const std::filesystem::path& path)
    : m_repo(git2wrap::repository::open(
          path.c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, nullptr))
{
  m_name = path.stem().string();
  read_file(m_owner, path, "owner");
  read_file(m_description, path, "description");

  for (auto it = m_repo.branch_begin(GIT_BRANCH_LOCAL);
       it != m_repo.branch_end();
       ++it)
  {
    m_branches.emplace_back(it->dup());
  }
}

void repository::read_file(std::string& out,
                           const std::filesystem::path& base,
                           const char* file)
{
  std::ifstream ifs(base / file);

  if (!ifs.is_open()) {
    ifs = base / ".git" / file;
  }

  if (ifs.is_open()) {
    std::getline(ifs, out, '\n');
  } else {
    out = "Unknown";
  }
}

}  // namespace startgit
