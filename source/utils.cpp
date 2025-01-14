#include <format>
#include <iomanip>
#include <sstream>

#include "utils.hpp"

#include <sys/stat.h>

namespace startgit
{

std::string long_to_string(int64_t date)
{
  std::stringstream strs;
  strs << std::put_time(std::gmtime(&date), "%Y-%m-%d %H:%M");  // NOLINT
  return strs.str();
}

std::string long_to_string(const git2wrap::time& time)
{
  std::stringstream strs;
  strs << std::put_time(std::gmtime(&time.time),  // NOLINT
                        "%a, %e %b %Y %H:%M:%S ");
  strs << (time.offset < 0 ? '-' : '+');
  strs << std::format("{:02}{:02}", time.offset / 60, time.offset % 60);
  return strs.str();
}
// NOLINTBEGIN
// clang-format off

void xmlencode(std::ostream& ost, const std::string& str)
{
    for (const char c: str) {
        switch(c) {
        case '<':  ost << "&lt;"; break;
        case '>':  ost << "&gt;"; break;
        case '\'': ost << "&#39;"; break;
        case '&':  ost << "&amp;"; break;
        case '"':  ost << "&quot;"; break;
        default:   ost << c;
        }
    }
}

std::string filemode(git2wrap::filemode_t filemode)
{
  std::string mode(10, '-');

  if (S_ISREG(filemode)) mode[0] = '-';
  else if (S_ISBLK(filemode)) mode[0] = 'b';
  else if (S_ISCHR(filemode)) mode[0] = 'c';
  else if (S_ISDIR(filemode)) mode[0] = 'd';
  else if (S_ISFIFO(filemode)) mode[0] = 'p';
  else if (S_ISLNK(filemode)) mode[0] = 'l';
  else if (S_ISSOCK(filemode)) mode[0] = 's';
  else mode[0] = '?';

  if (filemode & S_IRUSR) mode[1] = 'r';
  if (filemode & S_IWUSR) mode[2] = 'w';
  if (filemode & S_IXUSR) mode[3] = 'x';
  if (filemode & S_IRGRP) mode[4] = 'r';
  if (filemode & S_IWGRP) mode[5] = 'w';
  if (filemode & S_IXGRP) mode[6] = 'x';
  if (filemode & S_IROTH) mode[7] = 'r';
  if (filemode & S_IWOTH) mode[8] = 'w';
  if (filemode & S_IXOTH) mode[9] = 'x';

  if (filemode & S_ISUID) mode[3] = (mode[3] == 'x') ? 's' : 'S';
  if (filemode & S_ISGID) mode[6] = (mode[6] == 'x') ? 's' : 'S';
  if (filemode & S_ISVTX) mode[9] = (mode[9] == 'x') ? 't' : 'T';

  return mode;
}
// clang-format on
// NOLINTEND

}  // namespace startgit
