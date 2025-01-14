#include <functional>

#include "branch.hpp"

#include <git2wrap/revwalk.hpp>

#include "repository.hpp"

namespace startgit
{

branch::branch(git2wrap::branch brnch, repository& repo)
    : m_branch(std::move(brnch))
    , m_name(m_branch.get_name())
{
  git2wrap::revwalk rwalk(repo.get());

  const git2wrap::object obj = repo.get().revparse(m_name.c_str());
  rwalk.push(obj.get_id());

  while (auto commit = rwalk.next()) {
    m_commits.emplace_back(std::move(commit));
  }

  std::function<void(const git2wrap::tree&, const std::string& path)> traverse =
      [&](const auto& l_tree, const auto& path)
  {
    for (size_t i = 0; i < l_tree.get_entrycount(); i++) {
      const auto entry = l_tree.get_entry(i);
      const auto full_path =
          (!path.empty() ? path + "/" : "") + entry.get_name();

      switch (entry.get_type()) {
        case GIT_OBJ_BLOB:
          break;
        case GIT_OBJ_TREE:
          traverse(entry.to_tree(), full_path);
          continue;
        default:
          continue;
      }

      m_files.emplace_back(entry, full_path);
    }
  };

  traverse(get_last_commit().get_tree(), "");
}

}  // namespace startgit
