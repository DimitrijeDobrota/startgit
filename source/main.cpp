#include <filesystem>
#include <format>
#include <iostream>
#include <string>

#include <git2.h>
#include <git2wrap/error.hpp>
#include <git2wrap/libgit2.hpp>
#include <git2wrap/object.hpp>
#include <git2wrap/repository.hpp>
#include <git2wrap/revwalk.hpp>
#include <git2wrap/signature.hpp>
#include <poafloc/poafloc.hpp>

std::string long_to_string(int64_t date)
{
  std::stringstream strs;
  strs << std::put_time(std::gmtime(&date), "%Y-%m-%d %I:%M:%S %p");  // NOLINT
  return strs.str();
}

struct arguments_t
{
  std::filesystem::path output_dir = ".";
  std::vector<std::filesystem::path> repos;
  std::string url;
};

int parse_opt(int key, const char* arg, poafloc::Parser* parser)
{
  auto* args = static_cast<arguments_t*>(parser->input());
  switch (key) {
    case 'o':
      args->output_dir = arg;
      break;
    case 'u':
      args->url = arg;
      break;
    case poafloc::ARG:
      args->repos.emplace_back(arg);
      break;
    default:
      break;
  }
  return 0;
}

// NOLINTBEGIN
// clang-format off
static const poafloc::option_t options[] = {
    // {0, 0, 0, 0, "Output mode", 1},
    {"output", 'o', "DIR", 0, "Output directory"},
    {"url", 'u', "baseurl", 0, "Base URL to make links in the Atom feeds absolute"},
    {0},
};
// clang-format on

static const poafloc::arg_t arg {
    options,
    parse_opt,
    "config_file",
    "",
};
// NOLINTEND

int main(int argc, char* argv[])
{
  arguments_t args;

  if (poafloc::parse(&arg, argc, argv, 0, &args) != 0) {
    std::cerr << "There was an error while parsing arguments";
    return 1;
  }

  try {
    using namespace git2wrap;  // NOLINT

    const libgit2 libgit;

    for (const auto& repo_name : args.repos) {
      repository repo = repository::open(
          repo_name.c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, nullptr);

      for (auto it = repo.branch_begin(GIT_BRANCH_LOCAL);
           it != repo.branch_end();
           ++it)
      {
        const object obj = repo.revparse(it->get_name().c_str());
        std::cout << it->get_name() << " " << obj.get_id() << '\n';

        revwalk rwalk(repo);
        rwalk.push(obj.get_id());

        while (const auto commit = rwalk.next()) {
          std::cout << std::format("\t{}: {} ({})\n",
                                   long_to_string(commit.get_time()),
                                   commit.get_summary(),
                                   commit.get_author().get_name());
        }
      }
    }
  } catch (const git2wrap::error& err) {
    std::cerr << std::format("({}:{}) Error {}/{}: {}\n",
                             err.get_file(),
                             err.get_line(),
                             err.get_error(),
                             err.get_klass(),
                             err.get_message());
  }

  return 0;
}
