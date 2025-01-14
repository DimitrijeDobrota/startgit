#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

#include <git2wrap/error.hpp>
#include <git2wrap/libgit2.hpp>
#include <hemplate/classes.hpp>
#include <poafloc/poafloc.hpp>

#include "repository.hpp"
#include "utils.hpp"

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
             .add(
                 html::link({{"rel", "stylesheet"}, {"type", "text/css"}})
                     .set("href", "https://dimitrijedobrota.com/css/index.css"))
             .add(html::link({{"rel", "stylesheet"}, {"type", "text/css"}})
                      .set("href",
                           "https://dimitrijedobrota.com/css/colors.css"))
             // Icons
             .add(
                 html::link({{"rel", "icon"}, {"type", "image/png"}})
                     .set("sizes", "32x32")
                     .set("href",
                          "https://dimitrijedobrota.com/img/favicon-32x32.png"))
             .add(
                 html::link({{"rel", "icon"}, {"type", "image/png"}})
                     .set("sizes", "16x16")
                     .set(
                         "href",
                         "https://dimitrijedobrota.com/img/favicon-16x16.png"));
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
                 const std::string& branch_name,
                 const std::string& relpath = "./")
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
          .add(html::a("Log").set("href", relpath + "log.html"))
          .add(html::text(" | "))
          .add(html::a("Files").set("href", relpath + "files.html"))
          .add(html::text(" | "))
          .add(html::a("Refs").set("href", relpath + "refs.html"))
          .add(html::text(" | "))
          .add(html::a("README").set("href", "./"))
          .add(html::text(" | "))
          .add(html::a("LICENCE").set("href", "./"))
          .add(html::text(" | "))
          .add(dropdown));

  ost << html::table();
  ost << html::hr();
}

void write_commit_table(std::ostream& ost, const startgit::branch& branch)
{
  using namespace hemplate;  // NOLINT

  ost << html::table();
  ost << html::thead();
  ost << html::tr()
             .add(html::td("Date"))
             .add(html::td("Commit message"))
             .add(html::td("Author"))
             .add(html::td("Files"))
             .add(html::td("+"))
             .add(html::td("-"));
  ost << html::thead();
  ost << html::tbody();

  for (const auto& commit : branch.get_commits()) {
    const auto url = std::format("./commit/{}.html", commit.get_id());

    ost << html::tr()
               .add(html::td(commit.get_time()))
               .add(html::td().add(
                   html::a(commit.get_summary()).set("href", url)))
               .add(html::td(commit.get_author_name()))
               .add(html::td(commit.get_diff().get_files_changed()))
               .add(html::td(commit.get_diff().get_insertions()))
               .add(html::td(commit.get_diff().get_deletions()));
  }

  ost << html::tbody();
  ost << html::table();
}

void write_repo_table_entry(std::ostream& ost, const startgit::repository& repo)
{
  using namespace hemplate;  // NOLINT

  for (const auto& branch : repo.get_branches()) {
    if (branch.get_name() != "master") {
      continue;
    }

    const auto url = repo.get_name() + "/master/log.html";

    ost << html::tr()
               .add(html::td().add(html::a(repo.get_name()).set("href", url)))
               .add(html::td(repo.get_description()))
               .add(html::td(repo.get_owner()))
               .add(html::td(branch.get_commits()[0].get_author_time()));
    break;
  }
}

void write_repo_table(std::ostream& ost, const std::stringstream& index)
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

  ost << index.str();

  ost << html::tbody();
  ost << html::table();
}

void write_files_table(std::ostream& ost, const startgit::branch& branch)
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

  for (const auto& file : branch.get_files()) {
    const auto url = std::format("./file/{}.html", file.get_path());

    ost << html::tr()
               .add(html::td(file.get_filemode()))
               .add(html::td().add(html::a(file.get_path()).set("href", url)))
               .add(html::td("0"));
  }

  ost << html::tbody();
  ost << html::table();
}

void write_branch_table(std::ostream& ost,
                        const startgit::repository& repo,
                        const std::string& branch_name)
{
  using namespace hemplate;  // NOLINT

  ost << html::h2("Branches");
  ost << html::table();
  ost << html::thead();
  ost << html::tr()
             .add(html::td("&nbsp;"))
             .add(html::td("Name"))
             .add(html::td("Last commit date"))
             .add(html::td("Author"));
  ost << html::thead();
  ost << html::tbody();

  for (const auto& branch : repo.get_branches()) {
    const auto& last = branch.get_last_commit();
    const auto url = branch.get_name() != branch_name
        ? std::format("../{}/refs.html", branch.get_name())
        : "";

    ost << html::tr()
               .add(html::td(branch.get_name() == branch_name ? "*" : "&nbsp;"))
               .add(html::td().add(html::a(branch.get_name()).set("href", url)))
               .add(html::td(last.get_time()))
               .add(html::td(last.get_author_name()));
  }

  ost << html::tbody();
  ost << html::table();
}

void write_tag_table(std::ostream& ost, const startgit::repository& repo)
{
  using namespace hemplate;  // NOLINT

  ost << html::h2("Tags");
  ost << html::table();
  ost << html::thead();
  ost << html::tr()
             .add(html::td("&nbsp;"))
             .add(html::td("Name"))
             .add(html::td("Last commit date"))
             .add(html::td("Author"));
  ost << html::thead();
  ost << html::tbody();

  for (const auto& tag : repo.get_tags()) {
    ost << html::tr()
               .add(html::td("&nbsp;"))
               .add(html::td(tag.get_name()))
               .add(html::td(tag.get_time()))
               .add(html::td(tag.get_name()));
  }

  ost << html::tbody();
  ost << html::table();
}

void write_file_changes(std::ostream& ost, const startgit::diff& diff)
{
  using namespace hemplate;  // NOLINT

  ost << html::b("Diffstat:");
  ost << html::table() << html::tbody();

  for (const auto& delta : diff.get_deltas()) {
    static const char* marker = " ADMRC  T  ";

    const std::string link = std::format("#{}", delta->new_file.path);

    ost << html::tr()
               .add(html::td(std::string(1, marker[delta->status])))
               .add(html::td().add(
                   html::a(delta->new_file.path).set("href", link)))
               .add(html::td("|"))
               .add(html::td("..."));
  }

  ost << html::tbody() << html::table();
  ost << html::span(
      std::format("{} files changed, {} insertions(+), {} deletions(-)",
                  diff.get_files_changed(),
                  diff.get_insertions(),
                  diff.get_deletions()));
}

void write_file_diffs(std::ostream& ost, const startgit::diff& diff)
{
  using namespace hemplate;  // NOLINT

  for (const auto& delta : diff.get_deltas()) {
    const auto new_link = std::format("../file/{}.html", delta->new_file.path);
    const auto old_link = std::format("../file/{}.html", delta->old_file.path);

    ost << html::h3().set("id", delta->new_file.path);
    ost << "diff --git";
    ost << " a/" << html::a(delta->new_file.path).set("href", new_link);
    ost << " b/" << html::a(delta->old_file.path).set("href", old_link);
    ost << html::h3();

    for (const auto& hunk : delta.get_hunks()) {
      const std::string header(hunk->header);  // NOLINT

      ost << html::h4();
      ost << std::format("@@ -{},{} +{},{} @@ ",
                         hunk->new_start,
                         hunk->new_lines,
                         hunk->old_start,
                         hunk->old_lines);

      startgit::xmlencode(ost, header.substr(header.rfind('@') + 2));
      ost << html::h4();

      for (const auto& line : hunk.get_lines()) {
        if (line.get_origin() == '-') {
          ost << html::span().set(
              "style", "color: var(--theme_green); white-space: pre;");
        } else if (line.get_origin() == '+') {
          ost << html::span().set("style",
                                  "color: var(--theme_red); white-space: pre;");
        } else {
          ost << html::span().set("style", "white-space: pre;");
        }

        startgit::xmlencode(ost, line.get_content());
        ost << html::span();
      }
    }
  }
}

void write_commit_diff(std::ostream& ost, const startgit::commit& commit)
{
  using namespace hemplate;  // NOLINT

  ost << html::table() << html::tbody();

  const auto url = std::format("../commit/{}.html", commit.get_id());
  ost << html::tr()
             .add(html::td().add(html::b("commit")))
             .add(html::td().add(html::a(commit.get_id()).set("href", url)));

  if (commit.get_parentcount() > 0) {
    const auto purl = std::format("../commit/{}.html", commit.get_parent_id());
    ost << html::tr()
               .add(html::td().add(html::b("parent")))
               .add(html::td().add(
                   html::a(commit.get_parent_id()).set("href", purl)));
  }

  const auto mailto = std::string("mailto:") + commit.get_author_email();
  ost << html::tr();
  ost << html::td().add(html::b("author"));
  ost << html::td() << commit.get_author_name() << " &lt;";
  ost << html::a(commit.get_author_email()).set("href", mailto);
  ost << "&gt;" << html::td();
  ost << html::tr();

  ost << html::tr()
             .add(html::td().add(html::b("date")))
             .add(html::td(commit.get_author_time()));
  ost << html::tbody() << html::table();

  ost << html::br() << html::p().set("style", "white-space: pre;");
  startgit::xmlencode(ost, commit.get_message());
  ost << html::p();

  write_file_changes(ost, commit.get_diff());
  ost << html::hr();
  write_file_diffs(ost, commit.get_diff());
}

void write_footer(std::ostream& ost)
{
  using namespace hemplate;  // NOLINT

  ost << html::main();
  ost << html::div();
  ost << html::script(" ").set(
      "src", "https://www.dimitrijedobrota.com/scripts/main.js");
  ost << html::script(
      "function switchPage(value) {"
      "   let arr = window.location.href.split('/');"
      "   arr[4] = value;"
      "   history.replaceState(history.state, '', arr.join('/'));"
      "   location.reload();"
      "}");
  ost << html::style(
      "  table { "
      " margin-left: 0;"
      " background-color: inherit;"
      " border: none"
      "} select { "
      " color: var(--theme_fg1);"
      " background-color: inherit;"
      " border: 1px solid var(--theme_bg4);"
      "} select option {"
      " color: var(--theme_fg2) !important;"
      " background-color: var(--theme_bg3) !important;"
      "}");
  ost << html::body();
  ost << html::html();
}

struct arguments_t
{
  std::filesystem::path output_dir = ".";
  std::vector<std::filesystem::path> repos;
  std::string url;
};

void write_log(const std::filesystem::path& base,
               const startgit::repository& repo,
               const startgit::branch& branch)
{
  std::ofstream ofs(base / "log.html");

  write_header(ofs,
               repo.get_name(),
               branch.get_name(),
               repo.get_owner(),
               repo.get_description());
  write_title(ofs, repo, branch.get_name());
  write_commit_table(ofs, branch);
  write_footer(ofs);
}

void write_files(const std::filesystem::path& base,
                 const startgit::repository& repo,
                 const startgit::branch& branch)
{
  std::ofstream ofs(base / "files.html");

  write_header(ofs,
               repo.get_name(),
               branch.get_name(),
               repo.get_owner(),
               repo.get_description());
  write_title(ofs, repo, branch.get_name());
  write_files_table(ofs, branch);
  write_footer(ofs);
}

void write_refs(const std::filesystem::path& base,
                const startgit::repository& repo,
                const startgit::branch& branch)
{
  std::ofstream ofs(base / "refs.html");

  write_header(ofs,
               repo.get_name(),
               branch.get_name(),
               repo.get_owner(),
               repo.get_description());
  write_title(ofs, repo, branch.get_name());
  write_branch_table(ofs, repo, branch.get_name());
  write_tag_table(ofs, repo);
  write_footer(ofs);
}

void write_commits(const std::filesystem::path& base,
                   const startgit::repository& repo,
                   const startgit::branch& branch)
{
  for (const auto& commit : branch.get_commits()) {
    const std::string filename = commit.get_id() + ".html";
    std::ofstream ofs(base / filename);

    write_header(ofs,
                 repo.get_name(),
                 branch.get_name(),
                 "Dimitrije Dobrota",
                 commit.get_summary());
    write_title(ofs, repo, branch.get_name(), "../");
    write_commit_diff(ofs, commit);
    write_footer(ofs);
  }
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

  const git2wrap::libgit2 libgit;
  std::stringstream index;

  for (const auto& repo_path : args.repos) {
    try {
      const startgit::repository repo(repo_path);
      const std::filesystem::path base = args.output_dir / repo.get_name();
      std::filesystem::create_directory(base);

      for (const auto& branch : repo.get_branches()) {
        const std::filesystem::path base_branch = base / branch.get_name();
        std::filesystem::create_directory(base_branch);

        write_log(base_branch, repo, branch);
        write_files(base_branch, repo, branch);
        write_refs(base_branch, repo, branch);

        const std::filesystem::path commit = base_branch / "commit";
        std::filesystem::create_directory(commit);

        write_commits(commit, repo, branch);
      }

      write_repo_table_entry(index, repo);
    } catch (const git2wrap::error& err) {
      std::cerr << std::format("Warning: {} is not a repository\n",
                               repo_path.string());
    }
  }

  std::ofstream ofs(args.output_dir / "index.html");
  write_header(ofs,
               "Git repository",
               "~",
               "Dimitrije Dobrota",
               "Collection of all public git repositories");
  write_repo_table(ofs, index);
  write_footer(ofs);
  /*
  catch (const git2wrap::error& err) {
    std::cerr << std::format("({}:{}) Error {}/{}: {}\n",
                             err.get_file(),
                             err.get_line(),
                             err.get_error(),
                             err.get_klass(),
                             err.get_message());
  }
  */

  return 0;
}
