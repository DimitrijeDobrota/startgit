#include "commit.hpp"

#include <git2wrap/diff.hpp>
#include <git2wrap/signature.hpp>

#include "utils.hpp"

namespace startgit
{

commit::commit(git2wrap::commit cmmt)
    : m_commit(std::move(cmmt))
    , m_diff(m_commit)
{
}

std::string commit::get_id() const
{
  return m_commit.get_id().get_hex_string(shasize);
}

std::string commit::get_parent_id() const
{
  return m_commit.get_parent().get_id().get_hex_string(shasize);
}

size_t commit::get_parentcount() const
{
  return m_commit.get_parentcount();
}

std::string commit::get_summary() const
{
  std::string summary = m_commit.get_summary();

  static const int summary_limit = 50;
  if (summary.size() > summary_limit) {
    summary.resize(summary_limit);
    for (size_t i = summary.size() - 1; i >= summary.size() - 4; i--) {
      summary[i] = '.';
    }
  }

  return summary;
}

std::string commit::get_time() const
{
  return time_short(m_commit.get_author().get_time().time);
}

std::string commit::get_time_long() const
{
  return time_long(m_commit.get_author().get_time());
}

std::string commit::get_author_name() const
{
  return m_commit.get_author().get_name();
}

std::string commit::get_author_email() const
{
  return m_commit.get_author().get_name();
}

git2wrap::tree commit::get_tree() const
{
  return m_commit.get_tree();
}

std::string commit::get_message() const
{
  return m_commit.get_message();
}

}  // namespace startgit
