#pragma once

#include <ostream>
#include <string>

#include <git2wrap/types.hpp>

namespace startgit
{

std::string time_short(int64_t date);
std::string time_long(const git2wrap::time& time);
void xmlencode(std::ostream& ost, const std::string& str);
std::string filemode(git2wrap::filemode_t filemode);

}  // namespace startgit
