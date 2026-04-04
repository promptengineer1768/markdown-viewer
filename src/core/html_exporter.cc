#include "markdown/html_exporter.h"

#include <cctype>
#include <iterator>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "markdown/text_utils.h"

namespace markdown {
namespace {

std::string EscapeHtml(const std::string& text) {
  std::string escaped;
  escaped.reserve(text.size());
  for (char chr : text) {
    switch (chr) {
      case '&':
        escaped += "&amp;";
        break;
      case '<':
        escaped += "&lt;";
        break;
      case '>':
        escaped += "&gt;";
        break;
      case '"':
        escaped += "&quot;";
        break;
      case '\'':
        escaped += "&#x27;";
        break;
      case '/':
        escaped += "&#x2F;";
        break;
      default:
        const auto uchr = static_cast<unsigned char>(chr);
        if (uchr < 0x20 && uchr != '\t' && uchr != '\n' && uchr != '\r') {
          continue;
        }
        escaped.push_back(chr);
        break;
    }
  }
  return escaped;
}

std::string EscapeHtmlAttribute(const std::string& text) {
  std::string escaped;
  escaped.reserve(text.size());
  for (char chr : text) {
    switch (chr) {
      case '&':
        escaped += "&amp;";
        break;
      case '<':
        escaped += "&lt;";
        break;
      case '>':
        escaped += "&gt;";
        break;
      case '"':
        escaped += "&quot;";
        break;
      case '\'':
        escaped += "&#x27;";
        break;
      default:
        const auto uchr = static_cast<unsigned char>(chr);
        if (uchr < 0x20 && uchr != '\t') {
          continue;
        }
        escaped.push_back(chr);
        break;
    }
  }
  return escaped;
}

std::string RenderInlineNode(const InlineNode& node,
                             const std::string& base_url) {
  std::string text = EscapeHtml(node.text);

  auto replace_all = [](std::string& str, const std::string& from,
                        const std::string& to) {
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
      str.replace(pos, from.length(), to);
      pos += to.length();
    }
  };
  replace_all(text, ":smile:", "😊");
  replace_all(text, ":rocket:", "🚀");
  replace_all(text, ":check:", "✅");
  replace_all(text, ":warning:", "⚠️");

  auto is_safe_href = [&](const std::string& raw_url) {
    const std::string url = Trim(raw_url);
    if (url.empty()) {
      return false;
    }
    for (char chr : url) {
      const auto uchr = static_cast<unsigned char>(chr);
      if (uchr < 0x20 && uchr != '\t') {
        return false;
      }
    }
    const size_t scheme_separator = url.find(':');
    const size_t first_delimiter = url.find_first_of("/?#");
    if (scheme_separator == std::string::npos ||
        (first_delimiter != std::string::npos &&
         scheme_separator > first_delimiter)) {
      return true;
    }
    const std::string scheme = ToLowerAscii(url.substr(0, scheme_separator));
    return scheme == "http" || scheme == "https" || scheme == "mailto" ||
           scheme == "ftp";
  };

  auto is_relative = [&](const std::string& url_in) {
    const std::string url = Trim(url_in);
    if (url.empty()) {
      return false;
    }
    const size_t scheme_separator = url.find(':');
    const size_t first_delimiter = url.find_first_of("/?#");
    return (scheme_separator == std::string::npos ||
            (first_delimiter != std::string::npos &&
             scheme_separator > first_delimiter));
  };

  switch (node.type) {
    case InlineType::kEmphasis:
      return "<em>" + text + "</em>";
    case InlineType::kStrong:
      return "<strong>" + text + "</strong>";
    case InlineType::kCode:
      return "<code>" + text + "</code>";
    case InlineType::kStrikethrough:
      return "<del>" + text + "</del>";
    case InlineType::kLink:
      if (!is_safe_href(node.url)) {
        return text;
      }
      return "<a href=\"" + EscapeHtmlAttribute(Trim(node.url)) + "\">" + text +
             "</a>";
    case InlineType::kImage: {
      if (!is_safe_href(node.url)) {
        return "";
      }
      std::string final_url = Trim(node.url);
      if (is_relative(final_url) && !base_url.empty()) {
        final_url = base_url + final_url;
      }
      return R"(<img src=")" + EscapeHtmlAttribute(final_url) + R"(" alt=")" +
             EscapeHtmlAttribute(node.alt_text) +
             R"(" style="max-width: 100%;">)";
    }
    case InlineType::kText:
      return text;
  }
  return text;
}

std::string RenderInlineList(const std::vector<InlineNode>& nodes,
                             const std::string& base_url) {
  std::string html;
  for (const InlineNode& node : nodes) {
    html += RenderInlineNode(node, base_url);
  }
  return html;
}

std::string CollectInlineText(const std::vector<InlineNode>& nodes) {
  std::string text;
  for (const InlineNode& node : nodes) {
    if (!node.text.empty()) {
      text += node.text;
    } else if (!node.alt_text.empty()) {
      text += node.alt_text;
    }
  }
  return text;
}

std::string SlugifyHeadingText(const std::string& text) {
  std::string slug;
  bool previous_was_dash = false;
  for (unsigned char chr : text) {
    if (std::isalnum(chr) != 0) {
      slug.push_back(static_cast<char>(std::tolower(chr)));
      previous_was_dash = false;
      continue;
    }
    if (!previous_was_dash) {
      slug.push_back('-');
      previous_was_dash = true;
    }
  }
  while (!slug.empty() && slug.front() == '-') {
    slug.erase(slug.begin());
  }
  while (!slug.empty() && slug.back() == '-') {
    slug.pop_back();
  }
  if (slug.empty()) {
    return "section";
  }
  return slug;
}

std::vector<std::string> BuildHeadingIds(const Document& document) {
  std::vector<std::string> heading_ids(document.blocks.size());
  std::unordered_map<std::string, int> seen_slugs;
  for (size_t i = 0; i < document.blocks.size(); ++i) {
    const Block& block = document.blocks.at(i);
    if (block.type != BlockType::kHeading) {
      continue;
    }
    const std::string base_slug =
        SlugifyHeadingText(CollectInlineText(block.inlines));
    const int duplicate_count = seen_slugs[base_slug]++;
    heading_ids.at(i) = duplicate_count == 0
                            ? base_slug
                            : base_slug + "-" + std::to_string(duplicate_count);
  }
  return heading_ids;
}

}  // namespace

std::string RenderHtmlDocument(const Document& document,
                               const std::string& title, bool dark_theme,
                               const std::string& base_url) {
  const std::vector<std::string> heading_ids = BuildHeadingIds(document);
  std::ostringstream html;
  html << "<!DOCTYPE html>\n";
  html << "<html lang=\"en\">\n";
  html << "<head>\n";
  html << "  <meta charset=\"UTF-8\">\n";
  html << "  <meta name=\"viewport\" content=\"width=device-width, "
          "initial-scale=1.0\">\n";
  html << "  <title>" << EscapeHtml(title) << "</title>\n";
  if (!base_url.empty()) {
    html << "  <base href=\"" << EscapeHtml(base_url) << "\">\n";
  }
  html << "  <style>\n";
  if (dark_theme) {
    html << "    body { background: #1e1e1e; color: #ededed; "
            "font-family: Segoe UI, Arial, sans-serif; line-height: 1.6; "
            "margin: 0; max-width: none; padding: 0.75rem 1rem; }\n";
    html << "    a { color: #73b7ff; }\n";
    html << "    pre { background: #2a2a2a; border: 1px solid #3a3a3a; "
            "color: #f3f3f3; overflow-x: auto; padding: 1rem; position: "
            "relative; }\n";
    html << "    .code-lang { position: absolute; right: 0; top: 0; "
            "background: #444; "
            "color: #ccc; font-size: 0.7rem; padding: 0.1rem 0.4rem; }\n";
    html << "    code { background: #2f2f2f; border-radius: 3px; "
            "font-family: Consolas, monospace; padding: 0.1rem 0.25rem; }\n";
    html << "    blockquote { border-left: 4px solid #4a4a4a; color: #cfcfcf; "
            "margin: 1rem 0; padding-left: 1rem; }\n";
    html << "    table { border: 1px solid #3a3a3a; border-collapse: collapse; "
            "margin: 1rem 0; }\n";
    html << "    th, td { border: 1px solid #3a3a3a; padding: 0.4rem 0.8rem; "
            "}\n";
    html << "    th { background: #2a2a2a; }\n";
    html << "    .task-list-item { list-style-type: none; }\n";
    html << "    .footnote { font-size: 0.8rem; border-top: 1px solid #3a3a3a; "
            "margin-top: 2rem; }\n";
  } else {
    html << "    body { font-family: Segoe UI, Arial, sans-serif; "
            "line-height: 1.6; margin: 0; max-width: none; padding: 0.75rem "
            "1rem; }\n";
    html << "    pre { background: #f5f5f5; overflow-x: auto; padding: 1rem; "
            "position: relative; }\n";
    html << "    .code-lang { position: absolute; right: 0; top: 0; "
            "background: #ddd; "
            "color: #666; font-size: 0.7rem; padding: 0.1rem 0.4rem; }\n";
    html << "    code { font-family: Consolas, monospace; }\n";
    html << "    blockquote { border-left: 4px solid #d0d0d0; color: #555; "
            "margin: 1rem 0; padding-left: 1rem; }\n";
    html << "    table { border: 1px solid #ddd; border-collapse: collapse; "
            "margin: 1rem 0; }\n";
    html << "    th, td { border: 1px solid #ddd; padding: 0.4rem 0.8rem; }\n";
    html << "    th { background: #f2f2f2; }\n";
    html << "    .task-list-item { list-style-type: none; }\n";
    html << "    .footnote { font-size: 0.8rem; border-top: 1px solid #ddd; "
            "margin-top: 2rem; }\n";
  }
  html << "  </style>\n";
  html << "</head>\n";

  if (dark_theme) {
    html << "<body bgcolor=\"#1e1e1e\" text=\"#ededed\" link=\"#73b7ff\""
            " vlink=\"#9bc9ff\" alink=\"#73b7ff\">\n";
  } else {
    html << "<body bgcolor=\"#ffffff\" text=\"#1e1e1e\" link=\"#0066cc\""
            " vlink=\"#4f4fa8\" alink=\"#0066cc\">\n";
  }

  std::vector<BlockType> list_stack;
  auto close_lists = [&](size_t target_depth) {
    while (list_stack.size() > target_depth) {
      if (list_stack.back() == BlockType::kOrderedList) {
        html << "  </ol>\n";
      } else {
        html << "  </ul>\n";
      }
      list_stack.pop_back();
    }
  };

  for (size_t index = 0; index < document.blocks.size(); ++index) {
    const Block& block = document.blocks.at(index);
    const bool is_list = (block.type == BlockType::kListItem ||
                          block.type == BlockType::kOrderedList ||
                          block.type == BlockType::kTaskListItem);

    if (is_list) {
      size_t depth = static_cast<size_t>(block.indentation) + 1;
      if (depth > list_stack.size()) {
        if (block.type == BlockType::kOrderedList) {
          html << "  <ol"
               << (block.level > 1
                       ? " start=\"" + std::to_string(block.level) + "\""
                       : "")
               << ">\n";
          list_stack.push_back(BlockType::kOrderedList);
        } else {
          html << "  <ul>\n";
          list_stack.push_back(BlockType::kListItem);
        }
      } else if (depth < list_stack.size()) {
        close_lists(depth);
      }
    } else {
      close_lists(0);
    }

    switch (block.type) {
      case BlockType::kParagraph:
        html << "  <p>" << RenderInlineList(block.inlines, base_url)
             << "</p>\n";
        break;
      case BlockType::kHeading:
        html << "  <h" << block.level << " id=\"" << heading_ids.at(index)
             << "\">" << RenderInlineList(block.inlines, base_url) << "</h"
             << block.level << ">\n";
        break;
      case BlockType::kListItem:
      case BlockType::kOrderedList:
        html << "    <li>" << RenderInlineList(block.inlines, base_url)
             << "</li>\n";
        break;
      case BlockType::kTaskListItem:
        html << "    <li class=\"task-list-item\"><input type=\"checkbox\" "
                "disabled"
             << (block.task_checked ? " checked" : "") << "> "
             << RenderInlineList(block.inlines, base_url) << "</li>\n";
        break;
      case BlockType::kCodeBlock:
        html << "  <pre>";
        if (!block.language.empty()) {
          html << "<span class=\"code-lang\">" << EscapeHtml(block.language)
               << "</span>";
        }
        html << "<code>";
        for (auto line_it = block.lines.begin(); line_it != block.lines.end();
             ++line_it) {
          html << EscapeHtml(*line_it);
          if (std::next(line_it) != block.lines.end()) {
            html << '\n';
          }
        }
        html << "</code></pre>\n";
        break;
      case BlockType::kQuote:
        html << "  <blockquote>" << RenderInlineList(block.inlines, base_url)
             << "</blockquote>\n";
        break;
      case BlockType::kHorizontalRule:
        html << "  <hr>\n";
        break;
      case BlockType::kTable:
        html << "  <table>\n";
        for (size_t r = 0; r < block.table_cells.size(); ++r) {
          const auto& row = block.table_cells.at(r);
          html << "    <tr>\n";
          for (size_t c = 0; c < row.size(); ++c) {
            const char* tag = (r == 0) ? "th" : "td";
            html << "      <" << tag;
            if (c < block.alignments.size()) {
              if (block.alignments.at(c) == ColumnAlignment::kLeft) {
                html << " align=\"left\"";
              } else if (block.alignments.at(c) == ColumnAlignment::kCenter) {
                html << " align=\"center\"";
              } else if (block.alignments.at(c) == ColumnAlignment::kRight) {
                html << " align=\"right\"";
              }
            }
            html << ">" << RenderInlineList(row.at(c), base_url) << "</" << tag
                 << ">\n";
          }
          html << "    </tr>\n";
        }
        html << "  </table>\n";
        break;
      case BlockType::kFootnote:
        html << "  <div class=\"footnote\">[" << block.level << "] "
             << RenderInlineList(block.inlines, base_url) << "</div>\n";
        break;
      case BlockType::kTableOfContents:
        html << "  <div class=\"toc\"><strong>Table of Contents</strong><ul>";
        for (size_t toc_index = 0; toc_index < document.blocks.size();
             ++toc_index) {
          const Block& b = document.blocks.at(toc_index);
          if (b.type == BlockType::kHeading) {
            html << "<li><a href=\"#" << heading_ids.at(toc_index) << "\">"
                 << RenderInlineList(b.inlines, base_url) << "</a></li>";
          }
        }
        html << "</ul></div>\n";
        break;
      case BlockType::kDefinitionList:
        // Simple rendering for now
        html << "  <dl><dt><strong>"
             << RenderInlineList(block.inlines, base_url) << "</strong></dt>";
        for (const auto& line : block.lines) {
          html << "<dd>" << EscapeHtml(line) << "</dd>";
        }
        html << "</dl>\n";
        break;
    }
  }

  close_lists(0);
  html << "</body>\n\n</html>\n";
  return html.str();
}

}  // namespace markdown
