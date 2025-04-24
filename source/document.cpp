#include "document.hpp"

#include <hemplate/html.hpp>

#include "arguments.hpp"

namespace startgit
{

void document::render(std::ostream& ost, const content_t& content) const
{
  using namespace hemplate::html;  // NOLINT
  using hemplate::html::div;
  using hemplate::html::link;

  ost << transparent {
      doctype {},
      html {
          {{"lang", "en"}},
          head {
              title {m_title},

              // Meta tags
              meta {{
                  {"charset", "UTF-8"},
              }},
              meta {{
                  {"name", "author"},
                  {"content", m_author},
              }},
              meta {{
                  {"name", "description"},
                  {"content", m_desc},
              }},
              meta {{
                  {"content", "width=device-width, initial-scale=1"},
                  {"name", "viewport"},
              }},

              // Stylesheets
              link {{
                  {"rel", "stylesheet"},
                  {"type", "text/css"},
                  {"href", args.resource_url + "/css/index.css"},
              }},
              link {{
                  {"rel", "stylesheet"},
                  {"type", "text/css"},
                  {"href", args.resource_url + "/css/colors.css"},
              }},

              // Rss feed
              !m_has_feed ? text {} : [&]() -> element
              {
                return link {{
                    {"rel", "alternate"},
                    {"type", "application/atom+xml"},
                    {"title", "RSS feed"},
                    {"href", m_relpath + "rss.xml"},
                }};
              }(),

              // Atom feed
              !m_has_feed ? text {} : [&]() -> element
              {
                return link {{
                    {"rel", "alternate"},
                    {"type", "application/atom+xml"},
                    {"title", "Atom feed"},
                    {"href", m_relpath + "atom.xml"},
                }};
              }(),

              // Icons
              link {{
                  {"rel", "icon"},
                  {"type", "image/png"},
                  {"sizes", "32x32"},
                  {"href", args.resource_url + "/img/favicon-32x32.png"},
              }},
              link {{
                  {"rel", "icon"},
                  {"type", "image/png"},
                  {"sizes", "16x16"},
                  {"href", args.resource_url + "/img/favicon-16x16.png"},
              }},
          },
          body {
              input {{
                  {"type", "checkbox"},
                  {"id", "theme_switch"},
                  {"class", "theme_switch"},
              }},
              div {
                  {{"id", "content"}},
                  main {
                      label {{
                          {"class", "switch_label"},
                          {"for", "theme_switch"},
                      }},
                      content(),
                  },
              },
              script {{{"src", args.resource_url + "/scripts/main.js"}}},
              script {
                  "function switchPage(value) {"
                  "   let arr = window.location.href.split('/');"
                  "   arr[4] = value;"
                  "   history.replaceState(history.state, '', arr.join('/'));"
                  "   location.reload();"
                  "}"
              },
              style {"  table { "
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
                     "}"},
          },
      },
  };
}

}  // namespace startgit
