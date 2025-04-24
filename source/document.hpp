#pragma once

#include <ostream>
#include <string>

#include <hemplate/element.hpp>

#include "branch.hpp"
#include "repository.hpp"

namespace startgit
{

class document
{
  std::string m_title;
  std::string m_desc;
  std::string m_author;
  std::string m_relpath;
  bool m_has_feed;

  static auto form_title(const repository& repo, const branch& branch)
  {
    return std::format(
        "{} ({}) - {}",
        repo.get_name(),
        branch.get_name(),
        repo.get_description()
    );
  }

public:
  document(
      std::string_view title,
      std::string_view desc,
      std::string_view author,
      std::string_view relpath = "./",
      bool has_feed = true
  )
      : m_title(title)
      , m_desc(desc)
      , m_author(author)
      , m_relpath(relpath)
      , m_has_feed(has_feed)
  {
  }

  document(
      const repository& repo,
      const branch& branch,
      std::string_view desc,
      std::string_view relpath = "./",
      bool has_feed = true
  )
      : m_title(form_title(repo, branch))
      , m_desc(desc)
      , m_author(repo.get_owner())
      , m_relpath(relpath)
      , m_has_feed(has_feed)
  {
  }

  using content_t = std::function<hemplate::element()>;
  void render(std::ostream& ost, const content_t& content) const;
};

}  // namespace startgit
