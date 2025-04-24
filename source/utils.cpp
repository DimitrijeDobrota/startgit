#include <chrono>
#include <format>

#include "utils.hpp"

#include <sys/stat.h>

namespace startgit
{

auto sec_since_epoch(std::int64_t sec)
{
  return std::chrono::time_point_cast<std::chrono::seconds>(
      std::chrono::system_clock::from_time_t(time_t {0})
      + std::chrono::seconds(sec)
  );
}

std::string time_short(std::int64_t date)
{
  return std::format("{:%Y-%m-%d %H:%M}", sec_since_epoch(date));
}

std::string time_long(const git2wrap::time& time)
{
  return std::format(
      "{:%a, %e %b %Y %H:%M:%S} {}{:02}{:02}",
      sec_since_epoch(time.time),
      time.offset < 0 ? '-' : '+',
      time.offset / 60,  // NOLINT
      time.offset % 60  // NOLINT
  );
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

std::string xmlencode(const std::string& str)
{
    std::string res;

    res.reserve(str.size());
    for (const char c: str) {
        switch(c) {
        case '<':  res += "&lt;"; break;
        case '>':  res += "&gt;"; break;
        case '\'': res += "&#39;"; break;
        case '&':  res += "&amp;"; break;
        case '"':  res += "&quot;"; break;
        default:   res += c;
        }
    }

    return res;
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
