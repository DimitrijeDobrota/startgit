#include "diff.hpp"

namespace startgit
{

diff::diff(const git2wrap::commit& cmmt)
    : m_diff(nullptr, nullptr)
    , m_stats(nullptr)
{
  const auto ptree = cmmt.get_parentcount() > 0
      ? cmmt.get_parent().get_tree()
      : git2wrap::tree(nullptr, nullptr);

  git2wrap::diff_options opts;
  git_diff_init_options(&opts, GIT_DIFF_OPTIONS_VERSION);
  opts.flags = GIT_DIFF_DISABLE_PATHSPEC_MATCH | GIT_DIFF_IGNORE_SUBMODULES
      | GIT_DIFF_INCLUDE_TYPECHANGE;

  m_diff = git2wrap::diff::tree_to_tree(ptree, cmmt.get_tree(), &opts);
  m_stats = m_diff.get_stats();

  m_diff.foreach(file_cb, nullptr, hunk_cb, line_cb, this);
}

int diff::file_cb(const git_diff_delta* delta,
                  float /* progress */,
                  void* payload)
{
  diff& crnt = *reinterpret_cast<diff*>(payload);  // NOLINT
  crnt.m_deltas.emplace_back(delta);
  return 0;
}

int diff::hunk_cb(const git_diff_delta* /* delta */,
                  const git_diff_hunk* hunk,
                  void* payload)
{
  diff& crnt = *reinterpret_cast<diff*>(payload);  // NOLINT
  crnt.m_deltas.back().m_hunks.emplace_back(hunk);
  return 0;
}

int diff::line_cb(const git_diff_delta* /* delta */,
                  const git_diff_hunk* /* hunk */,
                  const git_diff_line* line,
                  void* payload)
{
  diff& crnt = *reinterpret_cast<diff*>(payload);  // NOLINT
  crnt.m_deltas.back().m_hunks.back().m_lines.emplace_back(line);
  return 0;
}

std::string diff::get_files_changed() const
{
  return std::to_string(m_stats.get_files_changed());
}

std::string diff::get_insertions() const
{
  return std::to_string(m_stats.get_insertions());
}

std::string diff::get_deletions() const
{
  return std::to_string(m_stats.get_deletions());
}

}  // namespace startgit
