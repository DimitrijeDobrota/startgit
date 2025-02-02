#include <cmath>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

#include <git2wrap/error.hpp>
#include <git2wrap/libgit2.hpp>
#include <hemplate/classes.hpp>
#include <poafloc/poafloc.hpp>

#include "arguments.hpp"
#include "common.hpp"
#include "html.hpp"
#include "repository.hpp"
#include "utils.hpp"

namespace startgit
{

void write_title(std::ostream& ost,
                 const repository& repo,
                 const branch& branch,
                 const std::string& relpath = "./")
{
  using namespace hemplate;  // NOLINT

  const auto dropdown = [&]()
  {
    auto span = html::span();
    span.add(html::label("Branch: ").set("for", "branch"));
    span.add(html::select(
        {{"id", "branch"}, {"onChange", "switchPage(this.value)"}}));

    for (const auto& c_branch : repo.get_branches()) {
      auto option = html::option(c_branch.get_name());
      option.set("value", c_branch.get_name());
      if (c_branch.get_name() == branch.get_name()) {
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

  ost << html::tr() << html::td();
  ost << html::a("Log").set("href", relpath + "log.html");
  ost << html::text(" | ")
      << html::a("Files").set("href", relpath + "files.html");
  ost << html::text(" | ")
      << html::a("Refs").set("href", relpath + "refs.html");

  for (const auto& file : branch.get_special()) {
    const auto filename = file.get_path().replace_extension("html").string();
    const auto name = file.get_path().replace_extension().string();
    ost << html::text(" | ") << html::a(name).set("href", relpath + filename);
  }

  ost << html::text(" | ") << dropdown;
  ost << html::td() << html::tr();

  ost << html::table();
  ost << html::hr();
}

void write_commit_table(std::ostream& ost, const branch& branch)
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

void write_files_table(std::ostream& ost, const branch& branch)
{
  using namespace hemplate;  // NOLINT

  ost << html::table();
  ost << html::thead();
  ost << html::tr()
             .add(html::td("Mode"))
             .add(html::td("Name"))
             .add(html::td("Size"));
  ost << html::thead();
  ost << html::tbody();

  for (const auto& file : branch.get_files()) {
    const auto url = std::format("./file/{}.html", file.get_path().string());
    const auto size = file.is_binary() ? std::format("{}B", file.get_size())
                                       : std::format("{}L", file.get_lines());

    ost << html::tr()
               .add(html::td(file.get_filemode()))
               .add(html::td().add(html::a(file.get_path()).set("href", url)))
               .add(html::td(size));
  }

  ost << html::tbody();
  ost << html::table();
}

void write_branch_table(std::ostream& ost,
                        const repository& repo,
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

void write_tag_table(std::ostream& ost, const repository& repo)
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
               .add(html::td(tag.get_author()));
  }

  ost << html::tbody();
  ost << html::table();
}

void write_file_changes(std::ostream& ost, const diff& diff)
{
  using namespace hemplate;  // NOLINT

  ost << html::b("Diffstat:");
  ost << html::table() << html::tbody();

  for (const auto& delta : diff.get_deltas()) {
    static const char* marker = " ADMRC  T  ";

    const std::string link = std::format("#{}", delta->new_file.path);

    uint32_t add = delta.get_adds();
    uint32_t del = delta.get_dels();
    const uint32_t changed = add + del;
    const uint32_t total = 80;
    if (changed > total) {
      const double percent = 1.0 * total / changed;

      if (add > 0) {
        add = static_cast<uint32_t>(std::lround(percent * add) + 1);
      }

      if (del > 0) {
        del = static_cast<uint32_t>(std::lround(percent * del) + 1);
      }
    }

    ost << html::tr()
               .add(html::td(std::string(1, marker[delta->status])))  // NOLINT
               .add(html::td().add(
                   html::a(delta->new_file.path).set("href", link)))
               .add(html::td("|"))
               .add(html::td()
                        .add(html::span()
                                 .add(html::text(std::string(add, '+')))
                                 .set("class", "add"))
                        .add(html::span()
                                 .add(html::text(std::string(del, '-')))
                                 .set("class", "del")));
  }

  ost << html::tbody() << html::table();
  ost << html::p(
      std::format("{} files changed, {} insertions(+), {} deletions(-)",
                  diff.get_files_changed(),
                  diff.get_insertions(),
                  diff.get_deletions()));
}

void write_file_diffs(std::ostream& ost, const diff& diff)
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
                         hunk->old_start,
                         hunk->old_lines,
                         hunk->new_start,
                         hunk->new_lines);

      xmlencode(ost, header.substr(header.rfind('@') + 2));
      ost << html::h4();

      ost << html::span().set("style", "white-space: pre");
      for (const auto& line : hunk.get_lines()) {
        auto div = html::div();
        if (line.is_add()) {
          div.set("class", "add");
        } else if (line.is_del()) {
          div.set("class", "del");
        }

        ost << div;
        xmlencode(ost, line.get_content());
        ost << div;
      }
      ost << html::span();
    }
  }
}

void write_commit_diff(std::ostream& ost, const commit& commit)
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
             .add(html::td(commit.get_time_long()));
  ost << html::tbody() << html::table();

  ost << html::br() << html::p().set("style", "white-space: pre;");
  xmlencode(ost, commit.get_message());
  ost << html::p();

  write_file_changes(ost, commit.get_diff());
  ost << html::hr();
  write_file_diffs(ost, commit.get_diff());
}

void write_file_title(std::ostream& ost, const file& file)
{
  using namespace hemplate;  // NOLINT

  ost << html::h3(std::format(
      "{} ({}B)", file.get_path().filename().string(), file.get_size()));
  ost << html::hr();
}

void write_file_content(std::ostream& ost, const file& file)
{
  using namespace hemplate;  // NOLINT

  if (file.is_binary()) {
    ost << html::h4("Binary file");
    return;
  }

  const std::string str(file.get_content(), file.get_size());
  std::stringstream sstr(str);

  std::string line;

  ost << html::span().set("style", "white-space: pre;");
  for (int count = 1; std::getline(sstr, line, '\n'); count++) {
    ost << std::format(
        R"(<a id="{}" href="#{}">{:5}</a>)", count, count, count);
    ost << "  ";
    xmlencode(ost, line);
    ost << '\n';
  }
  ost << html::span();
}

void write_html(std::ostream& ost, const file& file)
{
  static const auto process_output =
      +[](const MD_CHAR* str, MD_SIZE size, void* data)
  {
    std::ofstream& ofs = *static_cast<std::ofstream*>(data);
    ofs << std::string(str, size);
  };

  md_html(file.get_content(),
          static_cast<MD_SIZE>(file.get_size()),
          process_output,
          &ost,
          MD_DIALECT_GITHUB,
          0);
}

void write_log(const std::filesystem::path& base,
               const repository& repo,
               const branch& branch)
{
  std::ofstream ofs(base / "log.html");

  write_header(ofs, repo, branch, "Commit list");
  write_title(ofs, repo, branch);
  write_commit_table(ofs, branch);
  write_footer(ofs);
}

void write_file(const std::filesystem::path& base,
                const repository& repo,
                const branch& branch)
{
  std::ofstream ofs(base / "files.html");

  write_header(ofs, repo, branch, "File list");
  write_title(ofs, repo, branch);
  write_files_table(ofs, branch);
  write_footer(ofs);
}

void write_refs(const std::filesystem::path& base,
                const repository& repo,
                const branch& branch)
{
  std::ofstream ofs(base / "refs.html");

  write_header(ofs, repo, branch, "Refs list");
  write_title(ofs, repo, branch);
  write_branch_table(ofs, repo, branch.get_name());
  write_tag_table(ofs, repo);
  write_footer(ofs);
}

bool write_commits(const std::filesystem::path& base,
                   const repository& repo,
                   const branch& branch)
{
  bool changed = false;

  for (const auto& commit : branch.get_commits()) {
    const std::string file = base / (commit.get_id() + ".html");
    if (!args.force && std::filesystem::exists(file)) {
      break;
    }
    std::ofstream ofs(file);

    write_header(ofs, repo, branch, commit.get_summary(), "../");
    write_title(ofs, repo, branch, "../");
    write_commit_diff(ofs, commit);
    write_footer(ofs);
    changed = true;
  }

  return changed;
}

void write_files(const std::filesystem::path& base,
                 const repository& repo,
                 const branch& branch)
{
  for (const auto& file : branch.get_files()) {
    const std::filesystem::path path =
        base / (file.get_path().string() + ".html");
    std::filesystem::create_directories(path.parent_path());
    std::ofstream ofs(path);

    std::string relpath = "../";
    for (const char chr : file.get_path().string()) {
      if (chr == '/') {
        relpath += "../";
      }
    }

    write_header(ofs, repo, branch, file.get_path(), relpath);
    write_title(ofs, repo, branch, relpath);
    write_file_title(ofs, file);
    write_file_content(ofs, file);
    write_footer(ofs);
  }
}

void write_readme_licence(const std::filesystem::path& base,
                          const repository& repo,
                          const branch& branch)
{
  for (const auto& file : branch.get_special()) {
    std::ofstream ofs(base / file.get_path().replace_extension("html"));
    write_header(ofs, repo, branch, file.get_path());
    write_title(ofs, repo, branch);
    write_html(ofs, file);
    write_footer(ofs);
  }
}

void write_atom(std::ostream& ost,
                const branch& branch,
                const std::string& base_url)
{
  using namespace hemplate;  // NOLINT

  ost << atom::feed();
  ost << atom::title(args.title);
  ost << atom::subtitle(args.description);

  ost << atom::id(base_url + '/');
  ost << atom::updated(atom::format_time_now());
  ost << atom::author().add(atom::name(args.author));
  ost << atom::link(" ", {{"rel", "self"}, {"href", base_url + "/atom.xml"}});
  ost << atom::link(" ",
                    {{"href", args.resource_url},
                     {"rel", "alternate"},
                     {"type", "text/html"}});

  for (const auto& commit : branch.get_commits()) {
    const auto url =
        std::format("{}/commit/{}.html", base_url, commit.get_id());

    ost << atom::entry()
               .add(atom::id(url))
               .add(atom::updated(atom::format_time(commit.get_time_raw())))
               .add(atom::title(commit.get_summary()))
               .add(atom::link(" ").set("href", url))
               .add(atom::author()
                        .add(atom::name(commit.get_author_name()))
                        .add(atom::email(commit.get_author_email())))
               .add(atom::content(commit.get_message()));
  }

  ost << atom::feed();
}

void write_rss(std::ostream& ost,
               const branch& branch,
               const std::string& base_url)
{
  using namespace hemplate;  // NOLINT

  ost << xml();
  ost << rss::rss();
  ost << rss::channel();

  ost << rss::title(args.title);
  ost << rss::description(args.description);
  ost << rss::link(base_url + '/');
  ost << rss::generator("startgit");
  ost << rss::language("en-us");
  ost << rss::atomLink().set("href", base_url + "/atom.xml");

  for (const auto& commit : branch.get_commits()) {
    const auto url =
        std::format("{}/commit/{}.html", base_url, commit.get_id());

    ost << rss::item()
               .add(rss::title(commit.get_summary()))
               .add(rss::link(url))
               .add(rss::guid(url))
               .add(rss::pubDate(rss::format_time(commit.get_time_raw())))
               .add(rss::author(std::format("{} ({})",
                                            commit.get_author_email(),
                                            commit.get_author_name())));
  }

  ost << rss::channel();
  ost << rss::rss();
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
    case 'g':
      l_args->github = arg;
      break;
    case 'f':
      l_args->force = true;
      break;
    case 's': {
      std::stringstream sstream(arg);
      std::string crnt;

      l_args->special.clear();
      while (std::getline(sstream, crnt, ',')) {
        l_args->special.emplace(crnt);
      }

      break;
    }
    case poafloc::ARG:
      if (!l_args->repos.empty()) {
        std::cerr << std::format("Error: only one repository required\n");
        return -1;
      }

      try {
        l_args->repos.emplace_back(std::filesystem::canonical(arg));
      } catch (const std::filesystem::filesystem_error& arr) {
        std::cerr << std::format("Error: {} doesn't exist\n", arg);
        return -1;
      }
      break;
    case poafloc::END:
      if (l_args->repos.empty()) {
        std::cerr << std::format("Error: no repository provided\n");
        return -1;
      }
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
    {"special", 's', "NAME", 0, "Comma separated files to be rendered to html"},
    {"github", 'g', "USERNAME", 0, "Github username for url translation"},
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
    "repository",
    "",
};
// NOLINTEND

}  // namespace

int main(int argc, char* argv[])
{
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

    const repository repo(args.repos[0]);
    const std::filesystem::path base = args.output_dir / repo.get_name();
    std::filesystem::create_directory(base);

    for (const auto& branch : repo.get_branches()) {
      const std::filesystem::path base_branch = base / branch.get_name();
      std::filesystem::create_directory(base_branch);

      const std::filesystem::path commit = base_branch / "commit";
      std::filesystem::create_directory(commit);

      const bool changed = write_commits(commit, repo, branch);
      if (!args.force && !changed) {
        continue;
      };

      write_log(base_branch, repo, branch);
      write_file(base_branch, repo, branch);
      write_refs(base_branch, repo, branch);
      write_readme_licence(base_branch, repo, branch);

      const std::filesystem::path file = base_branch / "file";
      std::filesystem::create_directory(file);

      write_files(file, repo, branch);

      const std::string relative =
          std::filesystem::relative(base_branch, args.output_dir);
      const auto absolute = "https://git.dimitrijedobrota.com/" + relative;

      std::ofstream atom(base_branch / "atom.xml");
      write_atom(atom, branch, absolute);

      std::ofstream rss(base_branch / "rss.xml");
      write_rss(rss, branch, absolute);
    }
  } catch (const git2wrap::error<git2wrap::error_code_t::ENOTFOUND>& err) {
    std::cerr << std::format("Warning: {} is not a repository\n",
                             args.repos[0].string());
  } catch (const git2wrap::runtime_error& err) {
    std::cerr << std::format("Error (git2wrap): {}\n", err.what());
  } catch (const std::runtime_error& err) {
    std::cerr << std::format("Error: {}\n", err.what());
  } catch (...) {
    std::cerr << std::format("Unknown error\n");
  }

  return 0;
}
