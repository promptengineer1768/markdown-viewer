#include "markdown/markdown_parser.h"

#include <gtest/gtest.h>

namespace markdown {
namespace {

TEST(MarkdownParserTest, ParsesCommonBlockTypes) {
  const std::string source =
      "# Heading\n\nParagraph with **bold** text.\n\n- First\n> "
      "Quote\n\n```cpp\ncode\n```\n";

  const Document document = ParseMarkdown(source);

  ASSERT_EQ(document.blocks.size(), 5U);
  EXPECT_EQ(document.blocks[0].type, BlockType::kHeading);
  EXPECT_EQ(document.blocks[0].level, 1);
  EXPECT_EQ(document.blocks[1].type, BlockType::kParagraph);
  EXPECT_EQ(document.blocks[2].type, BlockType::kListItem);
  EXPECT_EQ(document.blocks[3].type, BlockType::kQuote);
  EXPECT_EQ(document.blocks[4].type, BlockType::kCodeBlock);
  EXPECT_EQ(document.blocks[4].language, "cpp");
  ASSERT_EQ(document.blocks[4].lines.size(), 1U);
  EXPECT_EQ(document.blocks[4].lines[0], "code");
}

TEST(MarkdownParserTest, ParsesListsAndTasks) {
  const std::string source =
      "1. First Ordered\n"
      "2. Second Ordered\n"
      "- [ ] Task Todo\n"
      "- [x] Task Done\n";
  const Document document = ParseMarkdown(source);

  ASSERT_EQ(document.blocks.size(), 4U);
  EXPECT_EQ(document.blocks[0].type, BlockType::kOrderedList);
  EXPECT_EQ(document.blocks[0].level, 1);
  EXPECT_EQ(document.blocks[1].level, 2);
  EXPECT_EQ(document.blocks[2].type, BlockType::kTaskListItem);
  EXPECT_EQ(document.blocks[2].task_checked, false);
  EXPECT_EQ(document.blocks[3].task_checked, true);
}

TEST(MarkdownParserTest, ParsesExtendedFormatting) {
  const std::string source =
      "~~Strikethrough~~ and ![Alt](img.png) and \\*escaped\\*.";
  const Document document = ParseMarkdown(source);

  ASSERT_EQ(document.blocks.size(), 1U);
  const auto& inlines = document.blocks[0].inlines;
  ASSERT_EQ(inlines.size(), 4U);
  EXPECT_EQ(inlines[0].type, InlineType::kStrikethrough);
  EXPECT_EQ(inlines[2].type, InlineType::kImage);
  EXPECT_EQ(inlines[2].alt_text, "Alt");
  EXPECT_EQ(inlines[2].url, "img.png");
  EXPECT_EQ(inlines[3].text, " and *escaped*.");
}

TEST(MarkdownParserTest, ParsesReferencesAndFootnotes) {
  const std::string source =
      "Check [ref][1] and [^2].\n\n"
      "[1]: https://google.com\n"
      "[^2]: Footnote text";
  const Document document = ParseMarkdown(source);

  // References are extracted but not in the block list
  // Footnotes are appended as blocks
  ASSERT_EQ(document.blocks.size(), 2U);
  EXPECT_EQ(document.blocks[0].inlines[1].type, InlineType::kLink);
  EXPECT_EQ(document.blocks[0].inlines[1].url, "https://google.com");
  EXPECT_EQ(document.blocks[1].type, BlockType::kFootnote);
  EXPECT_EQ(document.blocks[1].level, 2);
}

TEST(MarkdownParserTest, ParsesNestedLists) {
  const std::string source =
      "- Parent\n"
      "  - Child\n"
      "    - Grandchild\n";
  const Document document = ParseMarkdown(source);

  ASSERT_EQ(document.blocks.size(), 3U);
  EXPECT_EQ(document.blocks[0].indentation, 0);
  EXPECT_EQ(document.blocks[1].indentation, 1);
  EXPECT_EQ(document.blocks[2].indentation, 2);
}

TEST(MarkdownParserTest, KeepsIndentedContinuationInSameListItem) {
  const std::string source =
      "- First line\n"
      "  Second line\n";

  const Document document = ParseMarkdown(source);

  ASSERT_EQ(document.blocks.size(), 1U);
  EXPECT_EQ(document.blocks[0].type, BlockType::kListItem);
  ASSERT_FALSE(document.blocks[0].inlines.empty());
  EXPECT_NE(document.blocks[0].inlines[0].text.find("First line Second line"),
            std::string::npos);
}

TEST(MarkdownParserTest, ParsesTablesWithAlignment) {
  const std::string source =
      "| Type | Alignment |\n"
      "|:-----|:---------:|\n"
      "| Left | Center |\n";

  const Document document = ParseMarkdown(source);

  ASSERT_EQ(document.blocks.size(), 1U);
  EXPECT_EQ(document.blocks[0].type, BlockType::kTable);
  ASSERT_EQ(document.blocks[0].alignments.size(), 2U);
  EXPECT_EQ(document.blocks[0].alignments[0], ColumnAlignment::kLeft);
  EXPECT_EQ(document.blocks[0].alignments[1], ColumnAlignment::kCenter);
}

TEST(MarkdownParserTest, HandlesEmptyInput) {
  const Document document = ParseMarkdown("");
  EXPECT_TRUE(document.blocks.empty());
}

TEST(MarkdownParserTest, HandlesWhitespaceOnly) {
  const Document document = ParseMarkdown("   \n\t\n   ");
  EXPECT_TRUE(document.blocks.empty());
}

TEST(MarkdownParserTest, HandlesUnclosedCodeBlock) {
  const std::string source = "```cpp\ncode without closing";
  const Document document = ParseMarkdown(source);

  ASSERT_EQ(document.blocks.size(), 1U);
  EXPECT_EQ(document.blocks[0].type, BlockType::kCodeBlock);
  EXPECT_EQ(document.blocks[0].language, "cpp");
}

TEST(MarkdownParserTest, HandlesUnclosedBrackets) {
  const std::string source = "Text with [unclosed link";
  const Document document = ParseMarkdown(source);

  ASSERT_EQ(document.blocks.size(), 1U);
  EXPECT_EQ(document.blocks[0].type, BlockType::kParagraph);
}

TEST(MarkdownParserTest, HandlesMalformedImageSyntax) {
  const std::string source = "![missing closing](image.png";
  const Document document = ParseMarkdown(source);

  ASSERT_EQ(document.blocks.size(), 1U);
  EXPECT_EQ(document.blocks[0].type, BlockType::kParagraph);
}

TEST(MarkdownParserTest, HandlesDeeplyNestedLists) {
  std::string source;
  for (int i = 0; i < 10; ++i) {
    source += std::string(static_cast<std::string::size_type>(i) * 2U, ' ') +
              "- Item " + std::to_string(i) + "\n";
  }

  const Document document = ParseMarkdown(source);
  ASSERT_EQ(document.blocks.size(), 10U);

  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(document.blocks[i].indentation, i);
  }
}

TEST(MarkdownParserTest, HandlesMixedLineEndings) {
  const std::string source = "Line1\r\nLine2\nLine3\rLine4";
  const Document document = ParseMarkdown(source);

  ASSERT_GE(document.blocks.size(), 1U);
}

TEST(MarkdownParserTest, HandlesTableWithMalformedSeparator) {
  const std::string source =
      "| Header |\n"
      "| invalid |\n"
      "| Data |";
  const Document document = ParseMarkdown(source);

  ASSERT_GE(document.blocks.size(), 1U);
}

TEST(MarkdownParserTest, HandlesEmptyCodeBlock) {
  const std::string source = "```\n```";
  const Document document = ParseMarkdown(source);

  ASSERT_EQ(document.blocks.size(), 1U);
  EXPECT_EQ(document.blocks[0].type, BlockType::kCodeBlock);
  EXPECT_TRUE(document.blocks[0].lines.empty() ||
              (document.blocks[0].lines.size() == 1U &&
               document.blocks[0].lines[0].empty()));
}

TEST(MarkdownParserTest, HandlesVeryLongLine) {
  const std::string long_line(10000, 'a');
  const std::string source = long_line + "\n";

  const Document document = ParseMarkdown(source);
  ASSERT_EQ(document.blocks.size(), 1U);
  EXPECT_EQ(document.blocks[0].type, BlockType::kParagraph);
}

}  // namespace
}  // namespace markdown
