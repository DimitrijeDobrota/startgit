#pragma once

#include <string>

#include <git2wrap/tag.hpp>

namespace startgit
{

class tag
{
public:
  explicit tag(const git2wrap::tag& tagg);

  std::string get_name() const { return m_name; }
  std::string get_author() const { return m_author; }
  std::string get_time() const { return m_time; }

private:
  std::string m_name;
  std::string m_author;
  std::string m_time;
};

}  // namespace startgit
