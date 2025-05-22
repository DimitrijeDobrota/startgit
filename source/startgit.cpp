#include <cmath>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

#include <git2wrap/error.hpp>
#include <git2wrap/libgit2.hpp>
#include <hemplate/atom.hpp>
#include <hemplate/html.hpp>
#include <hemplate/rss.hpp>
#include <poafloc/poafloc.hpp>

#include "arguments.hpp"
#include "document.hpp"
#include "html.hpp"
#include "repository.hpp"
#include "utils.hpp"

using hemplate::element;
namespace
{

template<std::ranges::forward_range R>
element wtable(
    std::initializer_list<std::string_view> head_content,
    const R& range,
    based::Procedure<element, std::ranges::range_value_t<R>> auto proc
)
{
  using namespace hemplate::html;  // NOLINT

  return table {
      thead {
          tr {
              transform(
                  head_content,
                  [](const auto& elem)
                  {
                    return td {
                        elem,
                    };
                  }
              ),
          },
      },
      tbody {
          transform(range, proc),
      },
  };
}

}  // namespace

namespace startgit
{

element page_title(
    const repository& repo,
    const branch& branch,
    const std::string& relpath = "./"
)
{
  using namespace hemplate::html;  // NOLINT

  return element {
      table {
          tr {
              td {
                  h1 {repo.get_name()},
                  span {repo.get_description()},
              },
          },
          tr {
              td {
                  "git clone ",
                  aHref {repo.get_url(), repo.get_url()},
              },
          },
          tr {
              td {
                  aHref {relpath + "log.html", "Log"},
                  " | ",
                  aHref {relpath + "files.html", "Files"},
                  " | ",
                  aHref {relpath + "refs.html", "Refs"},
                  transform(
                      branch.get_special(),
                      [&](const auto& file)
                      {
                        auto path = file.get_path();
                        const auto filename =
                            path.replace_extension("html").string();
                        const auto name = path.replace_extension().string();

                        return element {
                            " | ",
                            aHref {relpath + filename, name},
                        };
                      }
                  ),
              },
          },
      },
      hr {},
  };
}

element commit_table(const branch& branch)
{
  using namespace hemplate::html;  // NOLINT

  return wtable(
      {"Date", "Commit message", "Author", "Files", "+", "-"},
      branch.get_commits(),
      [&](const auto& commit)
      {
        const auto idd = commit.get_id();
        const auto url = std::format("./commit/{}.html", idd);

        return tr {
            td {commit.get_time()},
            td {aHref {url, commit.get_summary()}},
            td {commit.get_author_name()},
            td {commit.get_diff().get_files_changed()},
            td {commit.get_diff().get_insertions()},
            td {commit.get_diff().get_deletions()},
        };
      }
  );
}

element files_table(const branch& branch)
{
  using namespace hemplate::html;  // NOLINT

  return wtable(
      {"Mode", "Name", "Size"},
      branch.get_files(),
      [&](const auto& file)
      {
        const auto path = file.get_path().string();
        const auto url = std::format("./file/{}.html", path);
        const auto size = file.is_binary()
            ? std::format("{}B", file.get_size())
            : std::format("{}L", file.get_lines());

        return tr {
            td {file.get_filemode()},
            td {aHref {url, path}},
            td {size},
        };
      }
  );
}

element branch_table(const repository& repo, const std::string& branch_name)
{
  using namespace hemplate::html;  // NOLINT

  return element {
      h2 {"Branches"},
      wtable(
          {"&nbsp;", "Name", "Last commit date", "Author"},
          repo.get_branches(),
          [&](const auto& branch)
          {
            const auto& last = branch.get_last_commit();
            const auto url = branch.get_name() != branch_name
                ? std::format("../{}/refs.html", branch.get_name())
                : "";
            const auto name = branch.get_name() == branch_name ? "*" : "&nbsp;";

            return tr {
                td {name},
                td {aHref {url, branch.get_name()}},
                td {last.get_time()},
                td {last.get_author_name()},
            };
          }
      ),
  };
}

element tag_table(const repository& repo)
{
  using namespace hemplate::html;  // NOLINT

  return element {
      h2 {"Tags"},
      wtable(
          {"&nbsp;", "Name", "Last commit date", "Author"},
          repo.get_tags(),
          [&](const auto& tag)
          {
            return tr {
                td {"&nbsp;"},
                td {tag.get_name()},
                td {tag.get_time()},
                td {tag.get_author()},
            };
          }
      ),
  };
}

element file_changes(const diff& diff)
{
  using namespace hemplate::html;  // NOLINT

  return element {
      b {"Diffstat:"},
      wtable(
          {},
          diff.get_deltas(),
          [&](const auto& delta)
          {
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

            return tr {
                td {std::string(1, marker[delta->status])},  // NOLINT
                td {aHref {link, delta->new_file.path}},
                td {"|"},
                td {
                    span {{{"class", "add"}}, std::string(add, '+')},
                    span {{{"class", "del"}}, std::string(del, '-')},
                },
            };
          }
      ),

      p {
          std::format(
              "{} files changed, {} insertions(+), {} deletions(-)",
              diff.get_files_changed(),
              diff.get_insertions(),
              diff.get_deletions()
          ),
      },
  };
}

element diff_hunk(const hunk& hunk)
{
  using namespace hemplate::html;  // NOLINT

  const std::string header(hunk->header);  // NOLINT
  return element {
      h4 {
          std::format(
              "@@ -{},{} +{},{} @@ ",
              hunk->old_start,
              hunk->old_lines,
              hunk->new_start,
              hunk->new_lines
          ),
          xmlencode(header.substr(header.rfind('@') + 2)),
      },
      span {
          transform(
              hunk.get_lines(),
              [](const auto& line) -> element
              {
                using hemplate::html::div;

                if (line.is_add()) {
                  return div {
                      {{"class", "inline add"}},
                      xmlencode(line.get_content()),
                  };
                }

                if (line.is_del()) {
                  return div {
                      {{"class", "inline del"}},
                      xmlencode(line.get_content()),
                  };
                }

                return div {
                    {{"class", "inline"}},
                    xmlencode(line.get_content()),
                };
              }
          ),
      },
  };
}

element file_diffs(const diff& diff)
{
  using namespace hemplate::html;  // NOLINT

  return transform(
      diff.get_deltas(),
      [](const auto& delta)
      {
        const auto& new_file = delta->new_file.path;
        const auto& old_file = delta->new_file.path;
        const auto new_link = std::format("../file/{}.html", new_file);
        const auto old_link = std::format("../file/{}.html", old_file);

        return element {
            h3 {
                {{"id", delta->new_file.path}},
                "diff --git",
                "a/",
                aHref {new_link, new_file},
                "b/",
                aHref {old_link, old_file},
            },
            transform(delta.get_hunks(), diff_hunk),
        };
      }
  );
}

element commit_diff(const commit& commit)
{
  using namespace hemplate::html;  // NOLINT

  const auto url = std::format("../commit/{}.html", commit.get_id());
  const auto mailto = std::string("mailto:") + commit.get_author_email();

  return element {
      table {
          tbody {
              tr {
                  td {b {"commit"}},
                  td {aHref {url, commit.get_id()}},
              },
              commit.get_parentcount() == 0 ? element {} : [&]() -> element
              {
                const auto purl =
                    std::format("../commit/{}.html", commit.get_parent_id());

                return tr {
                    td {b {"parent"}},
                    td {aHref {purl, commit.get_parent_id()}},
                };
              }(),
              tr {
                  td {b {"author"}},
                  td {
                      commit.get_author_name(),
                      "&lt;",
                      aHref {mailto, commit.get_author_email()},
                      "&gt;",
                  },
              },
              tr {
                  td {b {"date"}},
                  td {commit.get_time_long()},
              },
          },
      },
      br {},
      p {
          {{"class", "inline"}},
          xmlencode(commit.get_message()),
      },
      file_changes(commit.get_diff()),
      hr {},
      file_diffs(commit.get_diff()),
  };
}

element write_file_title(const file& file)
{
  using namespace hemplate::html;  // NOLINT

  const auto path = file.get_path().filename().string();

  return element {
      h3 {std::format("{} ({}B)", path, file.get_size())},
      hr {},
  };
}

element write_file_content(const file& file)
{
  using namespace hemplate::html;  // NOLINT

  if (file.is_binary()) {
    return h4("Binary file");
  }

  const std::string str(file.get_content(), file.get_size());
  std::stringstream sstr(str);

  std::vector<std::string> lines;
  std::string tmp;

  while (std::getline(sstr, tmp, '\n')) {
    lines.emplace_back(std::move(tmp));
  }

  int count = 0;
  return span {
      transform(
          lines,
          [&](const auto& line)
          {
            return hemplate::html::div {
                {{"class", "inline"}},
                std::format(
                    R"(<a id="{0}" href="#{0}">{0:5}</a> {1})",
                    count++,
                    xmlencode(line)
                )
            };
          }
      ),
  };
}

void write_log(
    const std::filesystem::path& base,
    const repository& repo,
    const branch& branch
)
{
  std::ofstream ofs(base / "log.html");
  document(repo, branch, "Commit list")
      .render(
          ofs,
          [&]()
          {
            return element {
                page_title(repo, branch),
                commit_table(branch),
            };
          }
      );
}

void write_file(
    const std::filesystem::path& base,
    const repository& repo,
    const branch& branch
)
{
  std::ofstream ofs(base / "files.html");
  document {repo, branch, "File list"}.render(
      ofs,
      [&]()
      {
        return element {
            page_title(repo, branch),
            files_table(branch),
        };
      }
  );
}

void write_refs(
    const std::filesystem::path& base,
    const repository& repo,
    const branch& branch
)
{
  std::ofstream ofs(base / "refs.html");
  document {repo, branch, "Refs list"}.render(
      ofs,
      [&]()
      {
        return element {
            page_title(repo, branch),
            branch_table(repo, branch.get_name()),
            tag_table(repo),
        };
      }
  );
}

bool write_commits(
    const std::filesystem::path& base,
    const repository& repo,
    const branch& branch
)
{
  bool changed = false;

  for (const auto& commit : branch.get_commits()) {
    const std::string file = base / (commit.get_id() + ".html");
    if (!args.force && std::filesystem::exists(file)) {
      break;
    }

    std::ofstream ofs(file);
    document {repo, branch, commit.get_summary(), "../"}.render(
        ofs,
        [&]()
        {
          return element {
              page_title(repo, branch, "../"),
              commit_diff(commit),
          };
        }
    );
    changed = true;
  }

  return changed;
}

void write_files(
    const std::filesystem::path& base,
    const repository& repo,
    const branch& branch
)
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

    document {repo, branch, file.get_path().string(), relpath}.render(
        ofs,
        [&]()
        {
          return element {
              page_title(repo, branch, relpath),
              write_file_title(file),
              write_file_content(file),
          };
        }
    );
  }
}

void write_readme_licence(
    const std::filesystem::path& base,
    const repository& repo,
    const branch& branch
)
{
  for (const auto& file : branch.get_special()) {
    std::ofstream ofs(base / file.get_path().replace_extension("html"));
    document {repo, branch, file.get_path().string()}.render(
        ofs,
        [&]()
        {
          std::string html;

          static const auto process_output =
              +[](const MD_CHAR* str, MD_SIZE size, void* data)
          {
            auto buffer = *static_cast<std::string*>(data);
            buffer += std::string(str, size);
          };

          md_html(
              file.get_content(),
              static_cast<MD_SIZE>(file.get_size()),
              process_output,
              &html,
              MD_DIALECT_GITHUB,
              0
          );
          return element {
              page_title(repo, branch),
              html,
          };
        }
    );
  }
}

void write_atom(
    std::ostream& ost, const branch& branch, const std::string& base_url
)
{
  using namespace hemplate::atom;  // NOLINT
  using hemplate::atom::link;

  ost << feed {
      title {args.title},
      subtitle {args.description},
      id {base_url + '/'},
      updated {format_time_now()},
      author {name {args.author}},
      linkSelf {base_url + "/atom.xml"},
      linkAlternate {args.resource_url},
      transform(
          branch.get_commits(),
          [&](const auto& commit)
          {
            const auto url =
                std::format("{}/commit/{}.html", base_url, commit.get_id());

            return entry {
                id {url},
                updated {format_time(commit.get_time_raw())},
                title {commit.get_summary()},
                linkHref {url},
                author {
                    name {commit.get_author_name()},
                    email {commit.get_author_email()},
                },
                content {commit.get_message()},
            };
          }
      ),
  };
}

void write_rss(
    std::ostream& ost, const branch& branch, const std::string& base_url
)
{
  using namespace hemplate::rss;  // NOLINT
  using hemplate::rss::link;
  using hemplate::rss::rss;

  ost << xml {};
  ost << rss {
      channel {
          title {args.title},
          description {args.description},
          link {base_url + '/'},
          generator {"startgit"},
          language {"en-us"},
          atomLink {base_url + "/atom.xml"},
          transform(
              branch.get_commits(),
              [&](const auto& commit)
              {
                const auto url =
                    std::format("{}/commit/{}.html", base_url, commit.get_id());

                return item {
                    title {commit.get_summary()},
                    link {url},
                    guid {url},
                    pubDate {format_time(commit.get_time_raw())},
                    author {std::format(
                        "{} ({})",
                        commit.get_author_email(),
                        commit.get_author_name()
                    )},
                };
              }
          ),
      },
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
    case 'g':
      l_args->github = arg;
      break;
    case 'f':
      l_args->force = true;
      break;
    case 's': {
      std::stringstream str(arg);
      std::string crnt;

      l_args->special.clear();
      while (std::getline(str, crnt, ',')) {
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

      // always update refs in case of a new branch or tag
      write_refs(base_branch, repo, branch);

      const bool changed = write_commits(commit, repo, branch);
      if (!args.force && !changed) {
        continue;
      };

      write_log(base_branch, repo, branch);
      write_file(base_branch, repo, branch);
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
  } catch (const git2wrap::error<git2wrap::error_code_t::enotfound>& err) {
    std::cerr << std::format(
        "Warning: {} is not a repository\n", args.repos[0].string()
    );
  } catch (const git2wrap::runtime_error& err) {
    std::cerr << std::format("Error (git2wrap): {}\n", err.what());
  } catch (const std::runtime_error& err) {
    std::cerr << std::format("Error: {}\n", err.what());
  } catch (...) {
    std::cerr << std::format("Unknown error\n");
  }

  return 0;
}
