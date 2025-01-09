#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

#include <git2.h>
#include <git2wrap/error.hpp>
#include <git2wrap/libgit2.hpp>
#include <git2wrap/object.hpp>
#include <git2wrap/repository.hpp>
#include <git2wrap/revwalk.hpp>
#include <git2wrap/signature.hpp>
#include <hemplate/classes.hpp>
#include <poafloc/poafloc.hpp>

#include "repository.hpp"

std::string long_to_string(int64_t date)
{
  std::stringstream strs;
  strs << std::put_time(std::gmtime(&date), "%Y-%m-%d %H:%M");  // NOLINT
  return strs.str();
}

void write_header(std::ostream& ost,
                  const std::string& repo,
                  const std::string& branch,
                  const std::string& author,
                  const std::string& description)
{
  using namespace hemplate;  // NOLINT

  const std::string name = repo + " - " + branch;

  ost << html::doctype();
  ost << html::html().set("lang", "en");
  ost << html::head()
             .add(html::title(name))
             // Meta tags
             .add(html::meta({{"charset", "UTF-8"}}))
             .add(html::meta({{"name", "author"}, {"content", author}}))
             .add(html::meta(
                 {{"name", "description"}, {"content", description}}))
             .add(
                 html::meta({{"content", "width=device-width, initial-scale=1"},
                             {"name", "viewport"}}))
             // Stylesheets
             .add(html::link({{"rel", "stylesheet"}, {"type", "text/css"}})
                      .set("href", "/css/index.css"))
             .add(html::link({{"rel", "stylesheet"}, {"type", "text/css"}})
                      .set("href", "/css/colors.css"))
             // Icons
             .add(html::link({{"rel", "icon"}, {"type", "image/png"}})
                      .set("sizes", "32x32")
                      .set("href", "/img/favicon-32x32.png"))
             .add(html::link({{"rel", "icon"}, {"type", "image/png"}})
                      .set("sizes", "16x16")
                      .set("href", "/img/favicon-16x16.png"));
  ost << html::body();
  ost << html::input()
             .set("type", "checkbox")
             .set("id", "theme_switch")
             .set("class", "theme_switch");

  ost << html::div().set("id", "content");
  ost << html::main();
  ost << html::label(" ")
             .set("for", "theme_switch")
             .set("class", "switch_label");
}

void write_title(std::ostream& ost,
                 const startgit::repository& repo,
                 const std::string& branch_name)
{
  using namespace hemplate;  // NOLINT

  ost << html::h1(repo.get_name());
  ost << html::h2(repo.get_description());

  ost << html::label("Branch: ").set("for", "branch");
  ost << html::select(
      {{"id", "branch"}, {"onChange", "switchPage(this.value)"}});
  for (const auto& branch : repo.get_branches()) {
    auto option = html::option(branch.get_name());
    option.set("value", branch.get_name());
    if (branch.get_name() == branch_name) {
      option.set("selected", "true");
    }
    ost << option;
  }
  ost << html::select();
}

void write_commit_table(std::ostream& ost, git2wrap::revwalk& rwalk)
{
  using namespace hemplate;  // NOLINT

  ost << html::table();
  ost << html::thead();
  ost << html::tr()
             .add(html::td("Date"))
             .add(html::td("Commit message"))
             .add(html::td("Author"));
  ost << html::thead();
  ost << html::tbody();

  while (const auto commit = rwalk.next()) {
    ost << html::tr()
               .add(html::td(long_to_string(commit.get_time())))
               .add(html::td().add(
                   html::a(commit.get_summary()).set("href", "./")))
               .add(html::td(commit.get_author().get_name()));
  }

  ost << html::tbody();
  ost << html::table();
}

void write_repo_table(std::ostream& ost,
                      const std::vector<startgit::repository>& repos)
{
  using namespace hemplate;  // NOLINT

  ost << html::table();
  ost << html::thead();
  ost << html::tr()
             .add(html::td("Name"))
             .add(html::td("Description"))
             .add(html::td("Owner"))
             .add(html::td("Last commit"));
  ost << html::thead();
  ost << html::tbody();

  for (const auto& repo : repos) {
    ost << html::tr()
               .add(html::td().add(
                   html::a(repo.get_name())
                       .set("href", repo.get_name() + "/master_log.html")))
               .add(html::td(repo.get_description()))
               .add(html::td(repo.get_owner()))
               .add(html::td(""));
  }

  ost << html::tbody();
  ost << html::table();
}

void write_footer(std::ostream& ost)
{
  using namespace hemplate;  // NOLINT

  ost << html::main();
  ost << html::div();
  ost << html::script(" ").set("src", "/scripts/main.js");
  ost << html::script(
      "function switchPage(value) { "
      "history.replaceState(history.state, '', `${value}_log.html`); "
      "location.reload();}");
  ost << html::body();
  ost << html::html();
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
      args->repos.emplace_back(std::filesystem::canonical(arg));
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
    {0, 0, 0, 0, "General information", 2},
    {"url", 'u', "BASEURL", 0, "Base URL to make links in the Atom feeds absolute"},
    {0, 0, 0, 0, "Informational Options", -1},
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
    const git2wrap::libgit2 libgit;
    std::vector<startgit::repository> repos;

    // open all repositories
    for (const auto& repo_path : args.repos) {
      try {
        repos.emplace_back(repo_path);
      } catch (const git2wrap::error& err) {
        std::cerr << std::format("Warning: {} is not a repository\n",
                                 repo_path.string());
      }
    }

    for (const auto& repo : repos) {
      for (const auto& branch : repo.get_branches()) {
        std::filesystem::create_directory(args.output_dir / repo.get_name());

        const std::string filename = branch.get_name() + "_log.html";
        std::ofstream ofs(args.output_dir / repo.get_name() / filename);

        git2wrap::revwalk rwalk(repo.get());

        const git2wrap::object obj =
            repo.get().revparse(branch.get_name().c_str());
        rwalk.push(obj.get_id());

        write_header(ofs,
                     repo.get_name(),
                     branch.get_name(),
                     repo.get_owner(),
                     repo.get_description());
        write_title(ofs, repo, branch.get_name());
        write_commit_table(ofs, rwalk);
        write_footer(ofs);
      }
    }

    // Build repo index
    std::ofstream ofs(args.output_dir / "index.html");

    write_header(ofs,
                 "Git repository",
                 "~",
                 "Dimitrije Dobrota",
                 "Collection of all public git repositories");
    write_repo_table(ofs, repos);
    write_footer(ofs);

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
