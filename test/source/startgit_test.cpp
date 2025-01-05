#include "lib.hpp"

auto main() -> int
{
  auto const lib = library {};

  return lib.name == "startgit" ? 0 : 1;
}
