#pragma once

#include <git2wrap/commit.hpp>
#include <git2wrap/tree.hpp>

#include "diff.hpp"

namespace startgit
{

class commit
{
public:
  explicit commit(git2wrap::commit cmmt);

  const auto& get() const { return m_commit; }
  const diff& get_diff() const { return m_diff; }

  std::string get_id() const;
  std::string get_parent_id() const;
  size_t get_parentcount() const;
  std::string get_summary() const;
  std::string get_time() const;
  std::string get_time_long() const;
  std::string get_author_name() const;
  std::string get_author_email() const;
  git2wrap::tree get_tree() const;
  std::string get_message() const;

private:
  static const int shasize = 40;

  git2wrap::commit m_commit;
  diff m_diff;
};

}  // namespace startgit
