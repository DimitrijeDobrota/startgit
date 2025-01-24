#include <array>
#include <format>
#include <functional>
#include <string>

#include <md4c-html.h>

#include "arguments.hpp"

constexpr bool isdigit(char chr)
{
  return '0' <= (chr) && (chr) <= '9';
}

constexpr bool islower(char chr)
{
  return 'a' <= (chr) && (chr) <= 'z';
}

constexpr bool isupper(char chr)
{
  return 'A' <= (chr) && (chr) <= 'Z';
}

constexpr bool isalnum(char chr)
{
  return islower(chr) || isupper(chr) || isdigit(chr);
}

class md_html
{
public:
  static bool need_html_esc(char chr)
  {
    return escape_map[static_cast<size_t>(chr)] & esc_flag::html;  // NOLINT
  }

  static bool need_url_esc(char chr)
  {
    return escape_map[static_cast<size_t>(chr)] & esc_flag::url;  // NOLINT
  }

  using append_fn = void (md_html::*)(const MD_CHAR*, MD_SIZE);

  void render_verbatim(const std::string& text);
  void render_verbatim(const MD_CHAR* text, MD_SIZE size);

  void render_html_escaped(const MD_CHAR* data, MD_SIZE size);
  void render_url_escaped(const MD_CHAR* data, MD_SIZE size);
  void render_utf8_codepoint(unsigned codepoint, append_fn fn_append);
  void render_entity(const MD_CHAR* text, MD_SIZE size, append_fn fn_append);
  void render_attribute(const MD_ATTRIBUTE* attr, append_fn fn_append);
  void render_open_ol_block(const MD_BLOCK_OL_DETAIL* det);
  void render_open_li_block(const MD_BLOCK_LI_DETAIL* det);
  void render_open_code_block(const MD_BLOCK_CODE_DETAIL* det);
  void render_open_td_block(const MD_CHAR* cell_type,
                            const MD_BLOCK_TD_DETAIL* det);
  void render_open_a_span(const MD_SPAN_A_DETAIL* det);
  void render_open_img_span(const MD_SPAN_IMG_DETAIL* det);
  void render_close_img_span(const MD_SPAN_IMG_DETAIL* det);
  void render_open_wikilink_span(const MD_SPAN_WIKILINK_DETAIL* det);

  void (*process_output)(const MD_CHAR*, MD_SIZE, void*);
  void* userdata;
  unsigned flags;
  int image_nesting_level;

private:
  enum esc_flag : unsigned char
  {
    html = 0x1U,
    url = 0x2U
  };

  static constexpr const std::array<unsigned char, 256> escape_map = []()
  {
    std::array<unsigned char, 256> res = {};
    const std::string url_esc = "~-_.+!*(),%#@?=;:/,+$";
    const std::string html_esc = "\"&<>";

    for (size_t i = 0; i < res.size(); ++i) {
      const auto chr = static_cast<char>(i);

      if (html_esc.find(chr) != std::string::npos) {
        res[i] |= esc_flag::html;  // NOLINT
      }

      if (!isalnum(chr) && url_esc.find(chr) == std::string::npos) {
        res[i] |= esc_flag::url;  // NOLINT
      }
    }

    return res;
  }();
};
/*****************************************
 ***  HTML rendering helper functions  ***
 *****************************************/

void md_html::render_verbatim(const MD_CHAR* text, MD_SIZE size)  // NOLINT
{
  process_output(text, size, userdata);
}

void md_html::render_verbatim(const std::string& text)  // NOLINT
{
  process_output(text.data(), static_cast<MD_SIZE>(text.size()), userdata);
}

void md_html::render_html_escaped(const MD_CHAR* data, MD_SIZE size)
{
  MD_OFFSET beg = 0;
  MD_OFFSET off = 0;

  while (true) {
    /* Optimization: Use some loop unrolling. */
    while (off + 3 < size && !md_html::need_html_esc(data[off + 0])  // NOLINT
           && !md_html::need_html_esc(data[off + 1])  // NOLINT
           && !md_html::need_html_esc(data[off + 2])  // NOLINT
           && !md_html::need_html_esc(data[off + 3]))  // NOLINT
    {
      off += 4;
    }

    while (off < size && !md_html::need_html_esc(data[off])) {  // NOLINT
      off++;
    }

    if (off > beg) {
      render_verbatim(data + beg, off - beg);  // NOLINT
    }

    if (off < size) {
      switch (data[off]) {  // NOLINT
        case '&':
          render_verbatim("&amp;");
          break;
        case '<':
          render_verbatim("&lt;");
          break;
        case '>':
          render_verbatim("&gt;");
          break;
        case '"':
          render_verbatim("&quot;");
          break;
      }
      off++;
    } else {
      break;
    }
    beg = off;
  }
}

void md_html::render_url_escaped(const MD_CHAR* data, MD_SIZE size)
{
  static const MD_CHAR* hex_chars = "0123456789ABCDEF";
  MD_OFFSET beg = 0;
  MD_OFFSET off = 0;

  while (true) {
    while (off < size && !md_html::need_url_esc(data[off])) {  // NOLINT
      off++;
    }

    if (off > beg) {
      render_verbatim(data + beg, off - beg);  // NOLINT
    }

    if (off < size) {
      std::array<char, 3> hex = {0};

      switch (data[off]) {  // NOLINT
        case '&':
          render_verbatim("&amp;");
          break;
        default:
          hex[0] = '%';
          hex[1] = hex_chars[(static_cast<unsigned>(data[off]) >> 4)  // NOLINT
                             & 0xf];  // NOLINT
          hex[2] = hex_chars[(static_cast<unsigned>(data[off]) >> 0)  // NOLINT
                             & 0xf];  // NOLINT
          render_verbatim(hex.data(), 3);
          break;
      }
      off++;
    } else {
      break;
    }

    beg = off;
  }
}

unsigned hex_val(char chr)
{
  if ('0' <= chr && chr <= '9') {
    return static_cast<unsigned>(chr - '0');  // NOLINT
  }

  if ('A' <= chr && chr <= 'Z') {
    return static_cast<unsigned>(chr - 'A' + 10);  // NOLINT
  }

  return static_cast<unsigned>(chr - 'a' + 10);  // NOLINT
}

// NOLINTBEGIN
void md_html::render_utf8_codepoint(unsigned codepoint, append_fn fn_append)
{
  static const MD_CHAR utf8_replacement_char[] = {
      char(0xef), char(0xbf), char(0xbd)};

  unsigned char utf8[4];
  size_t n;

  if (codepoint <= 0x7f) {
    n = 1;
    utf8[0] = static_cast<unsigned char>(codepoint);
  } else if (codepoint <= 0x7ff) {
    n = 2;
    utf8[0] = 0xc0 | ((codepoint >> 6) & 0x1f);
    utf8[1] = 0x80 + ((codepoint >> 0) & 0x3f);
  } else if (codepoint <= 0xffff) {
    n = 3;
    utf8[0] = 0xe0 | ((codepoint >> 12) & 0xf);
    utf8[1] = 0x80 + ((codepoint >> 6) & 0x3f);
    utf8[2] = 0x80 + ((codepoint >> 0) & 0x3f);
  } else {
    n = 4;
    utf8[0] = 0xf0 | ((codepoint >> 18) & 0x7);
    utf8[1] = 0x80 + ((codepoint >> 12) & 0x3f);
    utf8[2] = 0x80 + ((codepoint >> 6) & 0x3f);
    utf8[3] = 0x80 + ((codepoint >> 0) & 0x3f);
  }

  if (0 < codepoint && codepoint <= 0x10ffff) {
    std::invoke(fn_append,
                this,
                reinterpret_cast<char*>(utf8),
                static_cast<MD_SIZE>(n));  // NOLINT
  } else {
    std::invoke(fn_append, this, utf8_replacement_char, 3);
  }
}
// NOLINTEND

/* Translate entity to its UTF-8 equivalent, or output the verbatim one
 * if such entity is unknown (or if the translation is disabled). */
void md_html::render_entity(const MD_CHAR* text,
                            MD_SIZE size,
                            append_fn fn_append)
{
  /* We assume UTF-8 output is what is desired. */
  if (size > 3 && text[1] == '#') {  // NOLINT
    unsigned codepoint = 0;

    if (text[2] == 'x' || text[2] == 'X') {  // NOLINT
      /* Hexadecimal entity (e.g. "&#x1234abcd;")). */
      for (MD_SIZE idx = 3; idx < size - 1; idx++) {
        codepoint = 16 * codepoint + hex_val(text[idx]);  // NOLINT
      }
    } else {
      /* Decimal entity (e.g. "&1234;") */
      for (MD_SIZE idx = 2; idx < size - 1; idx++) {
        codepoint =
            10 * codepoint + static_cast<unsigned>(text[idx] - '0');  // NOLINT
      }
    }

    render_utf8_codepoint(codepoint, fn_append);
    return;
  }

  std::invoke(fn_append, this, text, size);
}

void md_html::render_attribute(const MD_ATTRIBUTE* attr, append_fn fn_append)
{
  for (int i = 0; attr->substr_offsets[i] < attr->size; i++) {  // NOLINT
    MD_TEXTTYPE type = attr->substr_types[i];  // NOLINT
    MD_OFFSET off = attr->substr_offsets[i];  // NOLINT
    MD_SIZE size = attr->substr_offsets[i + 1] - off;  // NOLINT
    const MD_CHAR* text = attr->text + off;  // NOLINT

    switch (type) {
      case MD_TEXT_NULLCHAR:
        render_utf8_codepoint(0x0000, &md_html::render_verbatim);
        break;
      case MD_TEXT_ENTITY:
        render_entity(text, size, fn_append);
        break;
      default:
        std::invoke(fn_append, this, text, size);
        break;
    }
  }
}

void md_html::render_open_ol_block(const MD_BLOCK_OL_DETAIL* det)
{
  if (det->start == 1) {
    render_verbatim("<ol>\n");
    return;
  }

  const auto buf = std::format(R"(<ol start="{}">\n)", det->start);
  render_verbatim(buf);
}

void md_html::render_open_li_block(const MD_BLOCK_LI_DETAIL* det)
{
  if (det->is_task != 0) {
    render_verbatim(
        "<li class=\"task-list-item\">"
        "<input type=\"checkbox\" "
        "class=\"task-list-item-checkbox\" disabled");
    if (det->task_mark == 'x' || det->task_mark == 'X') {
      render_verbatim(" checked");
    }
    render_verbatim(">");
  } else {
    render_verbatim("<li>");
  }
}

void md_html::render_open_code_block(const MD_BLOCK_CODE_DETAIL* det)
{
  render_verbatim("<pre><code");

  /* If known, output the HTML 5 attribute class="language-LANGNAME". */
  if (det->lang.text != nullptr) {
    render_verbatim(" class=\"language-");
    render_attribute(&det->lang, &md_html::render_html_escaped);
    render_verbatim("\"");
  }

  render_verbatim(">");
}

void md_html::render_open_td_block(const MD_CHAR* cell_type,
                                   const MD_BLOCK_TD_DETAIL* det)
{
  render_verbatim("<");
  render_verbatim(cell_type);

  switch (det->align) {
    case MD_ALIGN_LEFT:
      render_verbatim(" align=\"left\">");
      break;
    case MD_ALIGN_CENTER:
      render_verbatim(" align=\"center\">");
      break;
    case MD_ALIGN_RIGHT:
      render_verbatim(" align=\"right\">");
      break;
    default:
      render_verbatim(">");
      break;
  }
}

void md_html::render_open_a_span(const MD_SPAN_A_DETAIL* det)
{
  render_verbatim("<a href=\"");
  render_attribute(&det->href, &md_html::render_url_escaped);

  if (det->title.text != nullptr) {
    render_verbatim("\" title=\"");
    render_attribute(&det->title, &md_html::render_html_escaped);
  }

  render_verbatim("\">");
}

void md_html::render_open_img_span(const MD_SPAN_IMG_DETAIL* det)
{
  render_verbatim("<img src=\"");
  render_attribute(&det->src, &md_html::render_url_escaped);

  render_verbatim("\" alt=\"");
}

void md_html::render_close_img_span(const MD_SPAN_IMG_DETAIL* det)
{
  if (det->title.text != nullptr) {
    render_verbatim("\" title=\"");
    render_attribute(&det->title, &md_html::render_html_escaped);
  }

  render_verbatim("\">");
}

void md_html::render_open_wikilink_span(const MD_SPAN_WIKILINK_DETAIL* det)
{
  render_verbatim("<x-wikilink data-target=\"");
  render_attribute(&det->target, &md_html::render_html_escaped);

  render_verbatim("\">");
}

/**************************************
 ***  HTML renderer implementation  ***
 **************************************/

int enter_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata)
{
  static const MD_CHAR* head[] = {// NOLINT
                                  "<h1>",
                                  "<h2>",
                                  "<h3>",
                                  "<h4>",
                                  "<h5>",
                                  "<h6>"};
  auto* data = static_cast<class md_html*>(userdata);

  switch (type) {
    case MD_BLOCK_DOC: /* noop */
      break;
    case MD_BLOCK_QUOTE:
      data->render_verbatim("<blockquote>\n");
      break;
    case MD_BLOCK_UL:
      data->render_verbatim("<ul>\n");
      break;
    case MD_BLOCK_OL:
      data->render_open_ol_block(
          static_cast<const MD_BLOCK_OL_DETAIL*>(detail));
      break;
    case MD_BLOCK_LI:
      data->render_open_li_block(
          static_cast<const MD_BLOCK_LI_DETAIL*>(detail));
      break;
    case MD_BLOCK_HR:
      data->render_verbatim("<hr>\n");
      break;
    case MD_BLOCK_H:
      data->render_verbatim(
          head[static_cast<MD_BLOCK_H_DETAIL*>(detail)->level - 1]);  // NOLINT
      break;
    case MD_BLOCK_CODE:
      data->render_open_code_block(
          static_cast<const MD_BLOCK_CODE_DETAIL*>(detail));
      break;
    case MD_BLOCK_HTML: /* noop */
      break;
    case MD_BLOCK_P:
      data->render_verbatim("<p>");
      break;
    case MD_BLOCK_TABLE:
      data->render_verbatim("<table>\n");
      break;
    case MD_BLOCK_THEAD:
      data->render_verbatim("<thead>\n");
      break;
    case MD_BLOCK_TBODY:
      data->render_verbatim("<tbody>\n");
      break;
    case MD_BLOCK_TR:
      data->render_verbatim("<tr>\n");
      break;
    case MD_BLOCK_TH:
      data->render_open_td_block("th",
                                 static_cast<MD_BLOCK_TD_DETAIL*>(detail));
      break;
    case MD_BLOCK_TD:
      data->render_open_td_block("td",
                                 static_cast<MD_BLOCK_TD_DETAIL*>(detail));
      break;
  }

  return 0;
}

int leave_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata)
{
  static const MD_CHAR* head[] = {// NOLINT
                                  "</h1>\n",
                                  "</h2>\n",
                                  "</h3>\n",
                                  "</h4>\n",
                                  "</h5>\n",
                                  "</h6>\n"};
  auto* data = static_cast<class md_html*>(userdata);

  switch (type) {
    case MD_BLOCK_DOC: /*noop*/
      break;
    case MD_BLOCK_QUOTE:
      data->render_verbatim("</blockquote>\n");
      break;
    case MD_BLOCK_UL:
      data->render_verbatim("</ul>\n");
      break;
    case MD_BLOCK_OL:
      data->render_verbatim("</ol>\n");
      break;
    case MD_BLOCK_LI:
      data->render_verbatim("</li>\n");
      break;
    case MD_BLOCK_HR: /*noop*/
      break;
    case MD_BLOCK_H:
      data->render_verbatim(
          head[static_cast<MD_BLOCK_H_DETAIL*>(detail)->level - 1]);  // NOLINT
      break;
    case MD_BLOCK_CODE:
      data->render_verbatim("</code></pre>\n");
      break;
    case MD_BLOCK_HTML: /* noop */
      break;
    case MD_BLOCK_P:
      data->render_verbatim("</p>\n");
      break;
    case MD_BLOCK_TABLE:
      data->render_verbatim("</table>\n");
      break;
    case MD_BLOCK_THEAD:
      data->render_verbatim("</thead>\n");
      break;
    case MD_BLOCK_TBODY:
      data->render_verbatim("</tbody>\n");
      break;
    case MD_BLOCK_TR:
      data->render_verbatim("</tr>\n");
      break;
    case MD_BLOCK_TH:
      data->render_verbatim("</th>\n");
      break;
    case MD_BLOCK_TD:
      data->render_verbatim("</td>\n");
      break;
  }

  return 0;
}

int enter_span_callback(MD_SPANTYPE type, void* detail, void* userdata)
{
  auto* data = static_cast<class md_html*>(userdata);
  const bool inside_img = (data->image_nesting_level > 0);

  if (type == MD_SPAN_IMG) {
    data->image_nesting_level++;
  }

  if (inside_img) {
    return 0;
  }

  switch (type) {
    case MD_SPAN_EM:
      data->render_verbatim("<em>");
      break;
    case MD_SPAN_STRONG:
      data->render_verbatim("<strong>");
      break;
    case MD_SPAN_U:
      data->render_verbatim("<u>");
      break;
    case MD_SPAN_A:
      data->render_open_a_span(static_cast<MD_SPAN_A_DETAIL*>(detail));
      break;
    case MD_SPAN_IMG:
      data->render_open_img_span(static_cast<MD_SPAN_IMG_DETAIL*>(detail));
      break;
    case MD_SPAN_CODE:
      data->render_verbatim("<code>");
      break;
    case MD_SPAN_DEL:
      data->render_verbatim("<del>");
      break;
    case MD_SPAN_LATEXMATH:
      data->render_verbatim("<x-equation>");
      break;
    case MD_SPAN_LATEXMATH_DISPLAY:
      data->render_verbatim("<x-equation type=\"display\">");
      break;
    case MD_SPAN_WIKILINK:
      data->render_open_wikilink_span(
          static_cast<MD_SPAN_WIKILINK_DETAIL*>(detail));
      break;
  }

  return 0;
}

int leave_span_callback(MD_SPANTYPE type, void* detail, void* userdata)
{
  auto* data = static_cast<class md_html*>(userdata);

  if (type == MD_SPAN_IMG) {
    data->image_nesting_level--;
  }

  if (data->image_nesting_level > 0) {
    return 0;
  }

  switch (type) {
    case MD_SPAN_EM:
      data->render_verbatim("</em>");
      break;
    case MD_SPAN_STRONG:
      data->render_verbatim("</strong>");
      break;
    case MD_SPAN_U:
      data->render_verbatim("</u>");
      break;
    case MD_SPAN_A:
      data->render_verbatim("</a>");
      break;
    case MD_SPAN_IMG:
      data->render_close_img_span(static_cast<MD_SPAN_IMG_DETAIL*>(detail));
      break;
    case MD_SPAN_CODE:
      data->render_verbatim("</code>");
      break;
    case MD_SPAN_DEL:
      data->render_verbatim("</del>");
      break;
    case MD_SPAN_LATEXMATH: /*fall through*/
    case MD_SPAN_LATEXMATH_DISPLAY:
      data->render_verbatim("</x-equation>");
      break;
    case MD_SPAN_WIKILINK:
      data->render_verbatim("</x-wikilink>");
      break;
  }

  return 0;
}

int text_callback(MD_TEXTTYPE type,
                  const MD_CHAR* text,
                  MD_SIZE size,
                  void* userdata)
{
  auto* data = static_cast<class md_html*>(userdata);

  switch (type) {
    case MD_TEXT_NULLCHAR:
      data->render_utf8_codepoint(0x0000, &md_html::render_verbatim);
      break;
    case MD_TEXT_BR:
      data->render_verbatim(
          (data->image_nesting_level == 0 ? ("<br>\n") : " "));
      break;
    case MD_TEXT_SOFTBR:
      data->render_verbatim((data->image_nesting_level == 0 ? "\n" : " "));
      break;
    case MD_TEXT_HTML:
      data->render_verbatim(text, size);
      break;
    case MD_TEXT_ENTITY:
      data->render_entity(text, size, &md_html::render_html_escaped);
      break;
    default:
      data->render_html_escaped(text, size);
      break;
  }

  return 0;
}

namespace startgit
{

int md_html(const MD_CHAR* input,
            MD_SIZE input_size,
            void (*process_output)(const MD_CHAR*, MD_SIZE, void*),
            void* userdata,
            unsigned parser_flags,
            unsigned renderer_flags)
{
  class md_html render = {process_output, userdata, renderer_flags, 0};

  const MD_PARSER parser = {0,
                            parser_flags,
                            enter_block_callback,
                            leave_block_callback,
                            enter_span_callback,
                            leave_span_callback,
                            text_callback,
                            nullptr,
                            nullptr};

  return md_parse(input, input_size, &parser, &render);
}

}  // namespace startgit
