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

  ost << element {
      doctype {},
      html {
          {{"lang", "en"}},
          head {
              title {m_title},

              metaUTF8 {},
              metaName {"author", m_author},
              metaName {"description", m_desc},
              metaName {"viewport", "width=device-width, initial-scale=1"},

              linkStylesheet {args.resource_url + "/css/index.css"},
              linkStylesheet {args.resource_url + "/css/colors.css"},

              m_has_feed ? element {
                              linkRss {"RSS feed", m_relpath + "rss.xml"},
                              linkAtom {"Atom feed", m_relpath + "atom.xml"},
                          } : element {},

              linkIcon {"32x32", args.resource_url + "/img/favicon-32x32.png"},
              linkIcon {"16x16", args.resource_url + "/img/favicon-16x16.png"},
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
                     "} .inline {"
                     " white-space: pre;"
                     "}"},
          },
      },
  };
}

}  // namespace startgit
