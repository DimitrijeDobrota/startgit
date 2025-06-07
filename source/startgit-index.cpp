#include <format>
#include <fstream>
#include <iostream>

#include <git2wrap/error.hpp>
#include <git2wrap/libgit2.hpp>
#include <hemplate/html.hpp>
#include <poafloc/error.hpp>
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
  } catch (const git2wrap::error<git2wrap::error_code_t::enotfound>& err) {
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

int main(int argc, const char* argv[])
{
  using namespace hemplate::html;  // NOLINT
  using namespace startgit;  // NOLINT
  using namespace poafloc;  // NOLINT

  auto program = parser<arguments_t> {
      positional {
          argument_list {
              "repositories",
              &arguments_t::set_repository,
          },
      },
      group {
          "Output mode",
          direct {
              "o output",
              &arguments_t::output_dir,
              "DIR Output directory",
          },
          boolean {
              "f force",
              &arguments_t::force,
              "Force write even if file exists",
          },
      },
      group {
          "General Information",
          direct {
              "b base",
              &arguments_t::set_base,
              "URL Absolute destination",
          },
          direct {
              "r resources",
              &arguments_t::set_resource,
              "URL Location of styles and scripts",
          },
          direct {
              "a author",
              &arguments_t::author,
              "NAME Owner of the repository",
          },
          direct {
              "t title",
              &arguments_t::title,
              "TITLE Title for the index page",
          },
          direct {
              "d description",
              &arguments_t::description,
              "DESC Description for the index page",
          },
      },
  };

  try {
    program(args, argc, argv);
    if (args.repos.empty()) {
      return -1;
    }

    const git2wrap::libgit2 libgit;

    auto& output_dir = args.output_dir;
    std::filesystem::create_directories(output_dir);
    output_dir = std::filesystem::canonical(output_dir);

    std::ofstream ofs(args.output_dir / "index.html");
    const document doc {
        args.title, args.description, args.author, "./", /* has_feed = */ false
    };
    doc.render(ofs, write_table);
  } catch (const poafloc::runtime_error& err) {
    std::cerr << std::format("Error (poafloc): {}\n", err.what());
  } catch (const git2wrap::runtime_error& err) {
    std::cerr << std::format("Error (git2wrap): {}\n", err.what());
  } catch (const std::runtime_error& err) {
    std::cerr << std::format("Error: {}\n", err.what());
  } catch (...) {
    std::cerr << std::format("Unknown error\n");
  }

  return 0;
}
