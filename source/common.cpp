#include <format>

#include "common.hpp"

#include <hemplate/classes.hpp>

#include "arguments.hpp"

namespace startgit
{

void write_header(std::ostream& ost,
                  const std::string& title,
                  const std::string& description,
                  const std::string& author,
                  const std::string& relpath,
                  bool has_feed)
{
  using namespace hemplate;  // NOLINT

  ost << html::doctype();
  ost << html::html().set("lang", "en");
  ost << html::head();
  ost << html::title(title);

  // Meta tags
  ost << html::meta({{"charset", "UTF-8"}});
  ost << html::meta({{"name", "author"}, {"content", author}});
  ost << html::meta({{"name", "description"}, {"content", description}});

  ost << html::meta({{"content", "width=device-width, initial-scale=1"},
                     {"name", "viewport"}});

  // Stylesheets
  ost << html::link({{"rel", "stylesheet"}, {"type", "text/css"}})
             .set("href", args.resource_url + "/css/index.css");
  ost << html::link({{"rel", "stylesheet"}, {"type", "text/css"}})
             .set("href", args.resource_url + "/css/colors.css");

  if (has_feed) {
    // Rss feed
    ost << html::link({{"rel", "alternate"},
                       {"type", "application/atom+xml"},
                       {"title", "RSS feed"},
                       {"href", relpath + "rss.xml"}});
    // Atom feed
    ost << html::link({{"rel", "alternate"},
                       {"type", "application/atom+xml"},
                       {"title", "Atom feed"},
                       {"href", relpath + "atom.xml"}});
  }

  // Icons
  ost << html::link({{"rel", "icon"}, {"type", "image/png"}})
             .set("sizes", "32x32")
             .set("href", args.resource_url + "/img/favicon-32x32.png");
  ost << html::link({{"rel", "icon"}, {"type", "image/png"}})
             .set("sizes", "16x16")
             .set("href", args.resource_url + "/img/favicon-16x16.png");
  ost << html::head();
  ost << html::body();
  ost << html::input()
             .set("type", "checkbox")
             .set("id", "theme_switch")
             .set("class", "theme_switch");

  ost << html::div().set("id", "content");
  html::div().tgl_state();

  ost << html::main();
  ost << html::label(" ")
             .set("for", "theme_switch")
             .set("class", "switch_label");
}

void write_header(std::ostream& ost,
                  const repository& repo,
                  const branch& branch,
                  const std::string& description,
                  const std::string& relpath,
                  bool has_feed)
{
  write_header(ost,
               std::format("{} ({}) - {}",
                           repo.get_name(),
                           branch.get_name(),
                           repo.get_description()),
               description,
               repo.get_owner(),
               relpath,
               has_feed);
}

void write_footer(std::ostream& ost)
{
  using namespace hemplate;  // NOLINT

  ost << html::main();

  html::div().tgl_state();
  ost << html::div();

  const auto jss = args.resource_url + "/scripts/main.js";
  ost << html::script(" ").set("src", jss);
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
      "} .add {"
      " color: var(--theme_green);"
      "} .del {"
      " color: var(--theme_red);"
      "}");
  ost << html::body();
  ost << html::html();
}

}  // namespace startgit
