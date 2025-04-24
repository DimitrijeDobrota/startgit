#include <format>

#include "common.hpp"

#include <hemplate/html.hpp>

#include "arguments.hpp"

namespace startgit
{

void write_header(
    std::ostream& ost,
    const std::string& title_txt,
    const std::string& description,
    const std::string& author,
    const std::string& relpath,
    bool has_feed
)
{
  using namespace hemplate::html;  // NOLINT
  using hemplate::html::div;
  using hemplate::html::link;

  ost << doctype();
  ost << html({{"lang", "en"}});
  ost << head();
  ost << title(title_txt);

  // Meta tags
  ost << meta({{"charset", "UTF-8"}});
  ost << meta({{"name", "author"}, {"content", author}});
  ost << meta({{"name", "description"}, {"content", description}});

  ost << meta({
      {"content", "width=device-width, initial-scale=1"},
      {"name", "viewport"},
  });

  // Stylesheets
  ost << link({
      {"rel", "stylesheet"},
      {"type", "text/css"},
      {"href", args.resource_url + "/css/index.css"},
  });

  ost << link({
      {"rel", "stylesheet"},
      {"type", "text/css"},
      {"href", args.resource_url + "/css/colors.css"},
  });

  if (has_feed) {
    // Rss feed
    ost << link({
        {"rel", "alternate"},
        {"type", "application/atom+xml"},
        {"title", "RSS feed"},
        {"href", relpath + "rss.xml"},
    });

    // Atom feed
    ost << link({
        {"rel", "alternate"},
        {"type", "application/atom+xml"},
        {"title", "Atom feed"},
        {"href", relpath + "atom.xml"},
    });
  }

  // Icons
  ost << link({
      {"rel", "icon"},
      {"type", "image/png"},
      {"sizes", "32x32"},
      {"href", args.resource_url + "/img/favicon-32x32.png"},
  });
  ost << link({
      {"rel", "icon"},
      {"type", "image/png"},
      {"sizes", "16x16"},
      {"href", args.resource_url + "/img/favicon-16x16.png"},
  });
  ost << head();
  ost << body();
  ost << input({
      {"type", "checkbox"},
      {"id", "theme_switch"},
      {"class", "theme_switch"},
  });

  ost << div({{"id", "content"}});
  div().tgl_state();

  ost << main();
  ost << label({
      {"for", "theme_switch"},
      {"class", "switch_label"},
  });
}

void write_header(
    std::ostream& ost,
    const repository& repo,
    const branch& branch,
    const std::string& description,
    const std::string& relpath,
    bool has_feed
)
{
  write_header(
      ost,
      std::format(
          "{} ({}) - {}",
          repo.get_name(),
          branch.get_name(),
          repo.get_description()
      ),
      description,
      repo.get_owner(),
      relpath,
      has_feed
  );
}

void write_footer(std::ostream& ost)
{
  using namespace hemplate::html;  // NOLINT
  using hemplate::html::div;

  ost << main();

  div().tgl_state();
  ost << div();

  ost << script({{"src", args.resource_url + "/scripts/main.js"}});
  ost << script(
      "function switchPage(value) {"
      "   let arr = window.location.href.split('/');"
      "   arr[4] = value;"
      "   history.replaceState(history.state, '', arr.join('/'));"
      "   location.reload();"
      "}"
  );
  ost << style(
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
      "} .add {"
      " color: var(--theme_green);"
      "} .del {"
      " color: var(--theme_red);"
      "}"
  );
  ost << body();
  ost << html();
}

}  // namespace startgit
