#include "markdown/markdown_parser.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <ranges>
#include <sstream>
#include <utility>

#include "markdown/text_utils.h"

namespace markdown {
namespace {

struct ParserContext {
  std::map<std::string, std::string> references;
  std::map<std::string, std::string> footnotes;
};

bool IsHorizontalRule(const std::string& line) {
  const std::string trimmed = Trim(line);
  if (trimmed.size() < 3) {
    return false;
  }
  const char first = trimmed.front();
  if (first != '-' && first != '*' && first != '_') {
    return false;
  }
  return std::ranges::all_of(trimmed, [first](char ch) {
    return ch == first || ch == ' ' || ch == '\t';
  });
}

bool IsListMarker(const std::string& line) {
  return StartsWith(line, "- ") || StartsWith(line, "* ") ||
         StartsWith(line, "+ ");
}

std::vector<std::string> SplitTableCells(const std::string& line) {
  std::vector<std::string> cells;
  std::string trimmed = Trim(line);
  if (trimmed.empty()) {
    return cells;
  }

  if (trimmed.front() == '|') {
    trimmed = trimmed.substr(1);
  }
  if (!trimmed.empty() && trimmed.back() == '|') {
    trimmed.pop_back();
  }

  std::string current_cell;
  bool escaped = false;
  for (char ch : trimmed) {
    if (escaped) {
      current_cell.push_back(ch);
      escaped = false;
    } else if (ch == '\\') {
      escaped = true;
    } else if (ch == '|') {
      cells.push_back(Trim(current_cell));
      current_cell.clear();
    } else {
      current_cell.push_back(ch);
    }
  }
  cells.push_back(Trim(current_cell));
  return cells;
}

bool IsTableSeparator(const std::string& line, size_t* column_count) {
  const std::vector<std::string> cells = SplitTableCells(line);
  if (cells.empty()) {
    return false;
  }
  for (const auto& cell : cells) {
    if (cell.empty()) {
      return false;
    }
    if (cell.find_first_not_of("-: \t") != std::string::npos) {
      return false;
    }
    if (cell.find('-') == std::string::npos) {
      return false;
    }
  }
  if (column_count != nullptr) {
    *column_count = cells.size();
  }
  return true;
}

ColumnAlignment GetAlignment(const std::string& separator_cell) {
  const std::string cell = Trim(separator_cell);
  bool left = !cell.empty() && cell.front() == ':';
  bool right = !cell.empty() && cell.back() == ':';
  if (left && right) {
    return ColumnAlignment::kCenter;
  }
  if (right) {
    return ColumnAlignment::kRight;
  }
  if (left) {
    return ColumnAlignment::kLeft;
  }
  return ColumnAlignment::kNone;
}

std::vector<InlineNode> ParseInline(const std::string& text,
                                    const ParserContext& ctx) {
  std::vector<InlineNode> nodes;
  std::string buffer;

  auto flush_buffer = [&nodes, &buffer]() {
    if (!buffer.empty()) {
      nodes.push_back(InlineNode{.type = InlineType::kText,
                                 .text = buffer,
                                 .url = "",
                                 .alt_text = ""});
      buffer.clear();
    }
  };

  size_t index = 0;
  while (index < text.size()) {
    // Backslash Escaping
    if (text.at(index) == '\\' && index + 1 < text.size()) {
      buffer.push_back(text.at(index + 1));
      index += 2;
      continue;
    }

    // Strikethrough ~~
    if (index + 1 < text.size() && text.at(index) == '~' &&
        text.at(index + 1) == '~') {
      const size_t end = text.find("~~", index + 2);
      if (end != std::string::npos) {
        flush_buffer();
        nodes.push_back(
            InlineNode{.type = InlineType::kStrikethrough,
                       .text = text.substr(index + 2, end - index - 2),
                       .url = "",
                       .alt_text = ""});
        index = end + 2;
        continue;
      }
    }

    // Strong **
    if (index + 1 < text.size() && text.at(index) == '*' &&
        text.at(index + 1) == '*') {
      const size_t end = text.find("**", index + 2);
      if (end != std::string::npos) {
        flush_buffer();
        nodes.push_back(
            InlineNode{.type = InlineType::kStrong,
                       .text = text.substr(index + 2, end - index - 2),
                       .url = "",
                       .alt_text = ""});
        index = end + 2;
        continue;
      }
    }

    // Emphasis *
    if (text.at(index) == '*') {
      const size_t end = text.find('*', index + 1);
      if (end != std::string::npos) {
        flush_buffer();
        nodes.push_back(
            InlineNode{.type = InlineType::kEmphasis,
                       .text = text.substr(index + 1, end - index - 1),
                       .url = "",
                       .alt_text = ""});
        index = end + 1;
        continue;
      }
    }

    // Inline Code `
    if (text.at(index) == '`') {
      const size_t end = text.find('`', index + 1);
      if (end != std::string::npos) {
        flush_buffer();
        nodes.push_back(
            InlineNode{.type = InlineType::kCode,
                       .text = text.substr(index + 1, end - index - 1),
                       .url = "",
                       .alt_text = ""});
        index = end + 1;
        continue;
      }
    }

    // Images ![alt](url)
    if (text.at(index) == '!' && index + 1 < text.size() &&
        text.at(index + 1) == '[') {
      const size_t alt_end = text.find(']', index + 2);
      if (alt_end != std::string::npos && alt_end + 1 < text.size() &&
          text.at(alt_end + 1) == '(') {
        const size_t url_end = text.find(')', alt_end + 2);
        if (url_end != std::string::npos) {
          flush_buffer();
          InlineNode img;
          img.type = InlineType::kImage;
          img.alt_text = text.substr(index + 2, alt_end - index - 2);
          img.url = text.substr(alt_end + 2, url_end - alt_end - 2);
          nodes.push_back(std::move(img));
          index = url_end + 1;
          continue;
        }
      }
    }

    // Links [text](url) or [text][id]
    if (text.at(index) == '[') {
      const size_t text_end = text.find(']', index + 1);
      if (text_end != std::string::npos) {
        // Footnote ref [^id]
        if (index + 1 < text.size() && text.at(index + 1) == '^') {
          std::string id = text.substr(index + 2, text_end - index - 2);
          flush_buffer();
          nodes.push_back(InlineNode{.type = InlineType::kLink,
                                     .text = "[" + id + "]",
                                     .url = "#fn-" + id,
                                     .alt_text = ""});
          index = text_end + 1;
          continue;
        }

        if (text_end + 1 < text.size()) {
          if (text.at(text_end + 1) == '(') {
            const size_t url_end = text.find(')', text_end + 2);
            if (url_end != std::string::npos) {
              flush_buffer();
              nodes.push_back(InlineNode{
                  .type = InlineType::kLink,
                  .text = text.substr(index + 1, text_end - index - 1),
                  .url = text.substr(text_end + 2, url_end - text_end - 2),
                  .alt_text = ""});
              index = url_end + 1;
              continue;
            }
          } else if (text.at(text_end + 1) == '[') {
            const size_t id_end = text.find(']', text_end + 2);
            if (id_end != std::string::npos) {
              std::string id = text.substr(text_end + 2, id_end - text_end - 2);
              if (ctx.references.contains(id)) {
                flush_buffer();
                nodes.push_back(InlineNode{
                    .type = InlineType::kLink,
                    .text = text.substr(index + 1, text_end - index - 1),
                    .url = ctx.references.at(id),
                    .alt_text = ""});
                index = id_end + 1;
                continue;
              }
            }
          }
        }
      }
    }

    buffer.push_back(text.at(index));
    ++index;
  }

  flush_buffer();
  return nodes;
}

Block MakeInlineBlock(BlockType type, int level, const std::string& text,
                      const ParserContext& ctx) {
  Block block;
  block.type = type;
  block.level = level;
  block.inlines = ParseInline(text, ctx);
  return block;
}

}  // namespace

// Complexity note:
// - Time: O(n) for n input characters (single scan per pass over line content).
// - Space: O(n) for parsed output plus intermediate line/reference storage.
// The parser currently materializes the full input and AST in memory.
Document ParseMarkdown(const std::string& input) {
  Document document;
  const std::vector<std::string> lines = SplitLines(input);
  ParserContext ctx;

  // Pass 1: Extract References and Footnotes
  for (const auto& line : lines) {
    const std::string trimmed = Trim(line);
    if (trimmed.empty()) {
      continue;
    }

    // [^id]: text (Footnote)
    if (StartsWith(trimmed, "[^") && trimmed.find("]:") != std::string::npos) {
      size_t end = trimmed.find("]:");
      std::string id = trimmed.substr(2, end - 2);
      std::string text = Trim(trimmed.substr(end + 2));
      ctx.footnotes[id] = text;
    }
    // [id]: url (Reference)
    else if (!trimmed.empty() && trimmed.front() == '[' &&
             trimmed.find("]:") != std::string::npos) {
      size_t end = trimmed.find("]:");
      std::string id = trimmed.substr(1, end - 1);
      std::string url = Trim(trimmed.substr(end + 2));
      ctx.references[id] = url;
    }
  }

  // Pass 2: Main Parsing
  for (size_t i = 0; i < lines.size();) {
    const std::string raw_line = lines.at(i);
    const std::string trimmed = Trim(raw_line);

    if (trimmed.empty()) {
      ++i;
      continue;
    }

    // Skip Reference/Footnote definitions (already in ctx)
    if (trimmed.front() == '[' && trimmed.find("]:") != std::string::npos) {
      ++i;
      continue;
    }

    // TOC [TOC]
    if (trimmed == "[TOC]") {
      Block block;
      block.type = BlockType::kTableOfContents;
      document.blocks.push_back(std::move(block));
      ++i;
      continue;
    }

    // Fenced Code Block
    if (StartsWith(trimmed, "```")) {
      Block block;
      block.type = BlockType::kCodeBlock;
      block.language = trimmed.substr(3);
      ++i;
      while (i < lines.size() && !StartsWith(Trim(lines.at(i)), "```")) {
        block.lines.push_back(lines.at(i));
        ++i;
      }
      if (i < lines.size()) {
        ++i;
      }
      document.blocks.push_back(std::move(block));
      continue;
    }

    // Horizontal Rule
    if (IsHorizontalRule(trimmed)) {
      Block block;
      block.type = BlockType::kHorizontalRule;
      document.blocks.push_back(std::move(block));
      ++i;
      continue;
    }

    // Headings
    size_t heading_level = 0;
    while (heading_level < trimmed.size() && trimmed.at(heading_level) == '#') {
      ++heading_level;
    }
    if (heading_level > 0 && heading_level <= 6 &&
        heading_level < trimmed.size() && trimmed.at(heading_level) == ' ') {
      document.blocks.push_back(
          MakeInlineBlock(BlockType::kHeading, static_cast<int>(heading_level),
                          trimmed.substr(heading_level + 1), ctx));
      ++i;
      continue;
    }

    // Blockquote
    if (StartsWith(trimmed, "> ")) {
      std::ostringstream quote_text;
      quote_text << trimmed.substr(2);
      ++i;
      while (i < lines.size() && StartsWith(Trim(lines.at(i)), "> ")) {
        quote_text << " " << Trim(lines.at(i)).substr(2);
        ++i;
      }
      document.blocks.push_back(
          MakeInlineBlock(BlockType::kQuote, 0, quote_text.str(), ctx));
      continue;
    }

    // Lists (Ordered, Unordered, Task)
    size_t indentation = 0;
    while (
        indentation < raw_line.size() &&
        (raw_line.at(indentation) == ' ' || raw_line.at(indentation) == '\t')) {
      ++indentation;
    }
    size_t level = indentation / 2;  // Simple nesting rule

    if (IsListMarker(trimmed) ||
        (trimmed.size() > 2 && (std::isdigit(trimmed.at(0)) != 0) &&
         trimmed.find(". ") != std::string::npos)) {
      Block block;
      std::string content;
      if (IsListMarker(trimmed)) {
        block.type = BlockType::kListItem;
        content = trimmed.substr(2);
        // Task List Check
        if (StartsWith(content, "[ ] ")) {
          block.type = BlockType::kTaskListItem;
          block.task_checked = false;
          content = content.substr(4);
        } else if (StartsWith(content, "[x] ") || StartsWith(content, "[X] ")) {
          block.type = BlockType::kTaskListItem;
          block.task_checked = true;
          content = content.substr(4);
        }
      } else {
        block.type = BlockType::kOrderedList;
        size_t dot = trimmed.find(". ");
        block.level = std::stoi(trimmed.substr(0, dot));
        content = trimmed.substr(dot + 2);
      }

      // Multi-line continuation: Consume subsequent indented lines
      while (i + 1 < lines.size()) {
        const std::string& next_raw = lines.at(i + 1);
        const std::string next_trimmed = Trim(next_raw);
        if (next_trimmed.empty()) {
          break;
        }

        size_t next_indent = 0;
        while (next_indent < next_raw.size() &&
               (next_raw.at(next_indent) == ' ' ||
                next_raw.at(next_indent) == '\t')) {
          ++next_indent;
        }

        // Must be indented at least 1 space beyond parent list's base
        // indentation and NOT be another list marker or specialized block
        if (next_indent > indentation && !IsListMarker(next_trimmed) &&
            (next_trimmed.size() <= 2 ||
             !(std::isdigit(next_trimmed.at(0)) != 0) ||
             next_trimmed.find(". ") == std::string::npos)) {
          content += " " + next_trimmed;
          ++i;
        } else {
          break;
        }
      }

      block.indentation = static_cast<int>(level);
      block.inlines = ParseInline(Trim(content), ctx);
      document.blocks.push_back(std::move(block));
      ++i;
      continue;
    }

    // Table Detection
    size_t col_count = 0;
    if (trimmed.find('|') != std::string::npos && i + 1 < lines.size() &&
        IsTableSeparator(Trim(lines.at(i + 1)), &col_count)) {
      Block block;
      block.type = BlockType::kTable;
      const std::vector<std::string> headers = SplitTableCells(trimmed);
      const std::vector<std::string> separators =
          SplitTableCells(Trim(lines.at(i + 1)));
      for (const auto& s : separators) {
        block.alignments.push_back(GetAlignment(s));
      }
      std::vector<std::vector<InlineNode>> row;
      for (size_t c = 0; c < col_count; ++c) {
        row.push_back(
            ParseInline(c < headers.size() ? headers.at(c) : "", ctx));
      }
      block.table_cells.push_back(std::move(row));
      i += 2;
      while (i < lines.size()) {
        const std::string rl = Trim(lines.at(i));
        if (rl.empty() || rl.find('|') == std::string::npos) {
          break;
        }
        const std::vector<std::string> cells = SplitTableCells(rl);
        std::vector<std::vector<InlineNode>> data_row;
        for (size_t c = 0; c < col_count; ++c) {
          data_row.push_back(
              ParseInline(c < cells.size() ? cells.at(c) : "", ctx));
        }
        block.table_cells.push_back(std::move(data_row));
        ++i;
      }
      document.blocks.push_back(std::move(block));
      continue;
    }

    // Paragraph
    std::ostringstream para;
    para << trimmed;
    ++i;
    while (i < lines.size()) {
      const std::string next = Trim(lines.at(i));
      if (next.empty() || StartsWith(next, "#") || StartsWith(next, "```") ||
          IsListMarker(next) || StartsWith(next, "> ") ||
          IsHorizontalRule(next) ||
          (next.size() > 2 && (std::isdigit(next.at(0)) != 0) &&
           next.at(1) == '.')) {
        break;
      }
      para << " " << next;
      ++i;
    }
    document.blocks.push_back(
        MakeInlineBlock(BlockType::kParagraph, 0, para.str(), ctx));
  }

  // Final Pass: Append Footnotes
  if (!ctx.footnotes.empty()) {
    for (const auto& [id, text] : ctx.footnotes) {
      Block fn;
      fn.type = BlockType::kFootnote;
      fn.level = std::stoi(id);  // Simple numeric ID assumption
      fn.inlines = ParseInline(text, ctx);
      document.blocks.push_back(std::move(fn));
    }
  }

  return document;
}

}  // namespace markdown
