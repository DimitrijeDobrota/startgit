#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
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
#include <sys/stat.h>

#include "repository.hpp"

std::string long_to_string(int64_t date)
{
  std::stringstream strs;
  strs << std::put_time(std::gmtime(&date), "%Y-%m-%d %H:%M");  // NOLINT
  return strs.str();
}

// NOLINTBEGIN
// clang-format off
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

  const auto dropdown = [&]()
  {
    auto span = html::span();
    span.add(html::label("Branch: ").set("for", "branch"));
    span.add(html::select(
        {{"id", "branch"}, {"onChange", "switchPage(this.value)"}}));

    for (const auto& branch : repo.get_branches()) {
      auto option = html::option(branch.get_name());
      option.set("value", branch.get_name());
      if (branch.get_name() == branch_name) {
        option.set("selected", "true");
      }
      span.add(option);
    }

    span.add(html::select());
    return span;
  }();

  ost << html::table();
  ost << html::tr().add(html::td()
                            .add(html::h1(repo.get_name()))
                            .add(html::span(repo.get_description())));
  ost << html::tr().add(
      html::td()
          .add(html::text("git clone "))
          .add(html::a(repo.get_url()).set("href", repo.get_url())));
  ost << html::tr().add(
      html::td()
          .add(html::a("Log").set("href", branch_name + "_log.html"))
          .add(html::text(" | "))
          .add(html::a("Files").set("href", branch_name + "_files.html"))
          .add(html::text(" | "))
          .add(html::a("Refs").set("href", branch_name + "_refs.html"))
          .add(html::text(" | "))
          .add(html::a("README").set("href", "./"))
          .add(html::text(" | "))
          .add(html::a("LICENCE").set("href", "./"))
          .add(html::text(" | "))
          .add(dropdown));
  ost << html::table();
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

void write_files_table(std::ostream& ost, const git2wrap::tree& tree)
{
  using namespace hemplate;  // NOLINT

  ost << html::table();
  ost << html::thead();
  ost << html::tr()
             .add(html::td("Mode"))
             .add(html::td("Name"))
             .add(html::td("File"));
  ost << html::thead();
  ost << html::tbody();

  std::function<void(
      std::ostream&, const git2wrap::tree&, const std::string& path)>
      traverse = [&traverse](auto& l_ost, const auto& l_tree, const auto& path)
  {
    for (size_t i = 0; i < l_tree.get_entrycount(); i++) {
      const auto entry = l_tree.get_entry(i);
      const auto full_path =
          (!path.empty() ? path + "/" : "") + entry.get_name();

      switch (entry.get_type()) {
        case GIT_OBJ_BLOB:
          break;
        case GIT_OBJ_TREE:
          traverse(l_ost, entry.to_tree(), full_path);
          continue;
        default:
          continue;
      }

      l_ost << html::tr()
                   .add(html::td(filemode((entry.get_filemode()))))
                   .add(html::td().add(html::a(full_path).set("href", "./")))
                   .add(html::td("0"));
    }
  };
  traverse(ost, tree, "");

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
      "history.replaceState(history.state, '', value + "
      "window.location.href.substring(window.location.href.lastIndexOf('_'))); "
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

void write_log(const arguments_t& args,
               const startgit::repository& repo,
               const startgit::branch& branch)
{
  const std::string filename = branch.get_name() + "_log.html";
  std::ofstream ofs(args.output_dir / repo.get_name() / filename);

  git2wrap::revwalk rwalk(repo.get());

  const git2wrap::object obj = repo.get().revparse(branch.get_name().c_str());
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

void write_files(const arguments_t& args,
                 const startgit::repository& repo,
                 const startgit::branch& branch)
{
  const std::string filename = branch.get_name() + "_files.html";
  std::ofstream ofs(args.output_dir / repo.get_name() / filename);

  const git2wrap::object obj = repo.get().revparse(branch.get_name().c_str());
  const git2wrap::commit commit = repo.get().commit_lookup(obj.get_id());

  write_header(ofs,
               repo.get_name(),
               branch.get_name(),
               repo.get_owner(),
               repo.get_description());
  write_title(ofs, repo, branch.get_name());
  write_files_table(ofs, commit.get_tree());
  write_footer(ofs);
}

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

        write_log(args, repo, branch);
        write_files(args, repo, branch);
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
