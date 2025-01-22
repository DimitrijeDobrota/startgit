#pragma once

#include <string>

#include <git2wrap/commit.hpp>
#include <git2wrap/diff.hpp>

namespace startgit
{

struct line
{
  friend class diff;
  explicit line(const git_diff_line* lin)
      : m_content(lin->content, lin->content_len)
      , m_origin(lin->origin)
  {
  }

  const std::string& get_content() const { return m_content; }
  bool is_add() const { return m_origin == '+'; }
  bool is_del() const { return m_origin == '-'; }

private:
  std::string m_content;
  char m_origin;
};

struct hunk
{
  friend class diff;
  explicit hunk(const git_diff_hunk* dlt)
      : m_ptr(*dlt)
  {
  }

  const auto* operator->() const { return &m_ptr; }
  const auto& get_lines() const { return m_lines; }

private:
  git_diff_hunk m_ptr;
  std::vector<line> m_lines;
};

struct delta
{
  friend class diff;
  explicit delta(const git_diff_delta* dlt)
      : m_ptr(*dlt)
  {
  }
  const auto* operator->() const { return &m_ptr; }
  const auto& get_hunks() const { return m_hunks; }
  auto get_adds() const { return m_adds; }
  auto get_dels() const { return m_dels; }

private:
  git_diff_delta m_ptr;
  std::vector<hunk> m_hunks;
  uint32_t m_adds = 0;
  uint32_t m_dels = 0;
};

class diff
{
public:
  explicit diff(const git2wrap::commit& cmmt);

  std::string get_files_changed() const;
  std::string get_insertions() const;
  std::string get_deletions() const;

  const std::vector<delta>& get_deltas() const;

private:
  static int file_cb(const git_diff_delta* delta,
                     float progress,
                     void* payload);

  static int hunk_cb(const git_diff_delta* delta,
                     const git_diff_hunk* hunk,
                     void* payload);

  static int line_cb(const git_diff_delta* delta,
                     const git_diff_hunk* hunk,
                     const git_diff_line* line,
                     void* payload);

  git2wrap::diff m_diff;
  git2wrap::diff_stats m_stats;

  mutable std::vector<delta> m_deltas;
};

}  // namespace startgit
