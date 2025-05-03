#include <format>
#include <fstream>
#include <iostream>

#include <git2wrap/error.hpp>
#include <git2wrap/libgit2.hpp>
#include <hemplate/html.hpp>
#include <poafloc/poafloc.hpp>

#include "arguments.hpp"
#include "document.hpp"
#include "repository.hpp"
namespace startgit
{

hemplate::element write_table_row(const std::filesystem::path& repo_path)
{
  using namespace hemplate::html;  // NOLINT

  try {
    const repository repo(repo_path);

    for (const auto& branch : repo.get_branches()) {
      if (branch.get_name() != "master") {
        continue;
      }

      const auto url = repo.get_name() + "/master/log.html";
      return tr {
          td {aHref {url, repo.get_name()}},
          td {repo.get_description()},
          td {repo.get_owner()},
          td {branch.get_commits()[0].get_time()},
      };
    }

    std::cerr << std::format(
        "Warning: {} doesn't have master branch\n", repo.get_path().string()
    );
  } catch (const git2wrap::error<git2wrap::error_code_t::ENOTFOUND>& err) {
    std::cerr << std::format(
        "Warning: {} is not a repository\n", repo_path.string()
    );
  }

  return element {};
}

hemplate::element write_table()
{
  using namespace hemplate::html;  // NOLINT

  return element {
      h1 {args.title},
      p {args.description},
      table {
          thead {
              tr {
                  td {"Name"},
                  td {"Description"},
                  td {"Owner"},
                  td {"Last commit"},
              },
          },
          tbody {
              transform(args.repos, write_table_row),
          },
      }
  };
}

}  // namespace startgit

namespace
{

int parse_opt(int key, const char* arg, poafloc::Parser* parser)
{
  auto* l_args = static_cast<startgit::arguments_t*>(parser->input());
  switch (key) {
    case 'o':
      l_args->output_dir = arg;
      break;
    case 'b':
      l_args->base_url = arg;
      if (l_args->base_url.back() == '/') {
        l_args->base_url.pop_back();
      }
      break;
    case 'r':
      l_args->resource_url = arg;
      if (l_args->resource_url.back() == '/') {
        l_args->resource_url.pop_back();
      }
      break;
    case 'a':
      l_args->author = arg;
      break;
    case 'd':
      l_args->description = arg;
      break;
    case 'f':
      l_args->force = true;
      break;
    case poafloc::ARG:
      try {
        l_args->repos.emplace_back(std::filesystem::canonical(arg));
      } catch (const std::filesystem::filesystem_error& arr) {
        std::cerr << std::format("Warning: {} doesn't exist\n", arg);
      }
      break;
    case poafloc::END:
      if (l_args->repos.empty()) {
        std::cerr << std::format("Error: no repositories provided\n");
        return -1;
      }
      break;
    case poafloc::ERROR:
      poafloc::help(parser, stderr, poafloc::STD_ERR);
      break;
    default:
      break;
  }
  return 0;
}

// NOLINTBEGIN
// clang-format off
static const poafloc::option_t options[] = {
    {0, 0, 0, 0, "Output mode", 1},
    {"output", 'o', "DIR", 0, "Output directory"},
    {"force", 'f', 0, 0, "Force write even if file exists"},
    {0, 0, 0, 0, "General information", 2},
    {"base", 'b', "URL", 0, "Absolute destination URL"},
    {"resource", 'r', "URL", 0, "URL that houses styles and scripts"},
    {"author", 'a', "NAME", 0, "Owner of the repository"},
    {"title", 't', "TITLE", 0, "Title for the index page"},
    {"description", 'd', "DESC", 0, "Description for the index page"},
    {0, 0, 0, 0, "Informational Options", -1},
    {0},
};
// clang-format on

static const poafloc::arg_t arg {
    options,
    parse_opt,
    "repositories...",
    "",
};
// NOLINTEND

}  // namespace

int main(int argc, char* argv[])
{
  using namespace hemplate::html;  // NOLINT
  using namespace startgit;  // NOLINT

  if (poafloc::parse(&arg, argc, argv, 0, &args) != 0) {
    std::cerr << "There was an error while parsing arguments\n";
    return 1;
  }

  try {
    const git2wrap::libgit2 libgit;

    auto& output_dir = args.output_dir;
    std::filesystem::create_directories(output_dir);
    output_dir = std::filesystem::canonical(output_dir);

    std::ofstream ofs(args.output_dir / "index.html");
    const document doc {
        args.title, args.description, args.author, "./", /* has_feed = */ false
    };
    doc.render(ofs, write_table);

  } catch (const git2wrap::runtime_error& err) {
    std::cerr << std::format("Error (git2wrap): {}\n", err.what());
  } catch (const std::runtime_error& err) {
    std::cerr << std::format("Error: {}\n", err.what());
  } catch (...) {
    std::cerr << std::format("Unknown error\n");
  }

  return 0;
}
