#pragma once

#include <iostream>
#include <string>

#include "branch.hpp"
#include "repository.hpp"

namespace startgit
{

void write_header(std::ostream& ost,
                  const std::string& title_txt,
                  const std::string& description,
                  const std::string& author,
                  const std::string& relpath = "./",
                  bool has_feed = true);

void write_header(std::ostream& ost,
                  const repository& repo,
                  const branch& branch,
                  const std::string& description,
                  const std::string& relpath = "./",
                  bool has_feed = true);

void write_footer(std::ostream& ost);

}  // namespace startgit
