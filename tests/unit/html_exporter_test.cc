#include "markdown/html_exporter.h"

#include <gtest/gtest.h>

namespace markdown {
namespace {

TEST(HtmlExporterTest, IncludesHtml5HeaderAndTitle) {
  Document document;
  Block heading;
  heading.type = BlockType::kHeading;
  heading.level = 1;
  heading.inlines.push_back(InlineNode{.type = InlineType::kText,
                                       .text = "Hello World",
                                       .url = "",
                                       .alt_text = ""});
  document.blocks.push_back(heading);

  const std::string html = RenderHtmlDocument(document, "sample");

  EXPECT_NE(html.find("<!DOCTYPE html>"), std::string::npos);
  EXPECT_NE(html.find("<meta charset=\"UTF-8\">"), std::string::npos);
  EXPECT_NE(html.find("<meta name=\"viewport\" content=\"width=device-width, "
                      "initial-scale=1.0\">"),
            std::string::npos);
  EXPECT_NE(html.find("<title>sample</title>"), std::string::npos);
  EXPECT_NE(html.find("<h1"), std::string::npos);
  EXPECT_NE(html.find("Hello World</h1>"), std::string::npos);
}

TEST(HtmlExporterTest, WrapsListItemsInUnorderedList) {
  Document document;
  Block first;
  first.type = BlockType::kListItem;
  first.inlines.push_back(InlineNode{
      .type = InlineType::kText, .text = "First", .url = "", .alt_text = ""});
  document.blocks.push_back(first);

  Block second;
  second.type = BlockType::kListItem;
  second.inlines.push_back(InlineNode{
      .type = InlineType::kText, .text = "Second", .url = "", .alt_text = ""});
  document.blocks.push_back(second);

  const std::string html = RenderHtmlDocument(document, "items");

  EXPECT_NE(html.find("<ul>"), std::string::npos);
  EXPECT_NE(html.find("<li>First</li>"), std::string::npos);
  EXPECT_NE(html.find("<li>Second</li>"), std::string::npos);
}

TEST(HtmlExporterTest, RejectsUnsafeJavascriptLinks) {
  Document document;
  Block paragraph;
  paragraph.type = BlockType::kParagraph;
  paragraph.inlines.push_back(InlineNode{.type = InlineType::kLink,
                                         .text = "Click me",
                                         .url = "javascript:alert(1)",
                                         .alt_text = ""});
  document.blocks.push_back(paragraph);

  const std::string html = RenderHtmlDocument(document, "unsafe");

  EXPECT_EQ(html.find("javascript:alert(1)"), std::string::npos);
  EXPECT_EQ(html.find("<a href="), std::string::npos);
  EXPECT_NE(html.find("Click me"), std::string::npos);
}

TEST(HtmlExporterTest, PreservesSafeHttpsLinks) {
  Document document;
  Block paragraph;
  paragraph.type = BlockType::kParagraph;
  paragraph.inlines.push_back(InlineNode{.type = InlineType::kLink,
                                         .text = "Docs",
                                         .url = "https://example.com/docs",
                                         .alt_text = ""});
  document.blocks.push_back(paragraph);

  const std::string html = RenderHtmlDocument(document, "safe");

  // Title attribute was removed in previous step for style guide adherence
  EXPECT_NE(html.find("<a href=\"https://example.com/docs\">Docs</a>"),
            std::string::npos);
}

TEST(HtmlExporterTest, RendersTableToHtml) {
  Document document;
  Block table;
  table.type = BlockType::kTable;
  table.alignments = {ColumnAlignment::kLeft, ColumnAlignment::kCenter};

  std::vector<std::vector<InlineNode>> row1;
  row1.push_back({InlineNode{
      .type = InlineType::kText, .text = "Head1", .url = "", .alt_text = ""}});
  row1.push_back({InlineNode{
      .type = InlineType::kText, .text = "Head2", .url = "", .alt_text = ""}});
  table.table_cells.push_back(row1);

  std::vector<std::vector<InlineNode>> row2;
  row2.push_back({InlineNode{
      .type = InlineType::kText, .text = "Data1", .url = "", .alt_text = ""}});
  row2.push_back({InlineNode{
      .type = InlineType::kText, .text = "Data2", .url = "", .alt_text = ""}});
  table.table_cells.push_back(row2);

  document.blocks.push_back(table);

  const std::string html = RenderHtmlDocument(document, "table");

  EXPECT_NE(html.find("<table>"), std::string::npos);
  EXPECT_NE(html.find("<th align=\"left\">Head1</th>"), std::string::npos);
  EXPECT_NE(html.find("<th align=\"center\">Head2</th>"), std::string::npos);
  EXPECT_NE(html.find("<td align=\"left\">Data1</td>"), std::string::npos);
  EXPECT_NE(html.find("<td align=\"center\">Data2</td>"), std::string::npos);
}

TEST(HtmlExporterTest, AppliesDarkThemeBodyAttributes) {
  Document document;
  const std::string html = RenderHtmlDocument(document, "dark", true);

  EXPECT_NE(html.find("bgcolor=\"#1e1e1e\""), std::string::npos);
  EXPECT_NE(html.find("text=\"#ededed\""), std::string::npos);
}

TEST(HtmlExporterTest, EscapesSingleQuote) {
  Document document;
  Block paragraph;
  paragraph.type = BlockType::kParagraph;
  paragraph.inlines.push_back(InlineNode{.type = InlineType::kText,
                                         .text = "It's a test",
                                         .url = "",
                                         .alt_text = ""});
  document.blocks.push_back(paragraph);

  const std::string html = RenderHtmlDocument(document, "test");
  EXPECT_NE(html.find("&#x27;"), std::string::npos);
}

TEST(HtmlExporterTest, EscapesForwardSlash) {
  Document document;
  Block paragraph;
  paragraph.type = BlockType::kParagraph;
  paragraph.inlines.push_back(InlineNode{.type = InlineType::kText,
                                         .text = "</script>",
                                         .url = "",
                                         .alt_text = ""});
  document.blocks.push_back(paragraph);

  const std::string html = RenderHtmlDocument(document, "test");
  EXPECT_NE(html.find("&#x2F;"), std::string::npos);
}

TEST(HtmlExporterTest, RemovesControlCharacters) {
  Document document;
  Block paragraph;
  paragraph.type = BlockType::kParagraph;
  paragraph.inlines.push_back(InlineNode{.type = InlineType::kText,
                                         .text = "Hello\x01\x02World",
                                         .url = "",
                                         .alt_text = ""});
  document.blocks.push_back(paragraph);

  const std::string html = RenderHtmlDocument(document, "test");
  EXPECT_EQ(html.find("\x01"), std::string::npos);
  EXPECT_EQ(html.find("\x02"), std::string::npos);
  EXPECT_NE(html.find("HelloWorld"), std::string::npos);
}

TEST(HtmlExporterTest, PreservesNewlinesInCodeBlocks) {
  Document document;
  Block code;
  code.type = BlockType::kCodeBlock;
  code.lines.emplace_back("line1\nline2");
  document.blocks.push_back(code);

  const std::string html = RenderHtmlDocument(document, "test");
  EXPECT_NE(html.find("line1\nline2"), std::string::npos);
}

TEST(HtmlExporterTest, RejectsDataUriScheme) {
  Document document;
  Block paragraph;
  paragraph.type = BlockType::kParagraph;
  paragraph.inlines.push_back(
      InlineNode{.type = InlineType::kLink,
                 .text = "Data Link",
                 .url = "data:text/html,<script>alert(1)</script>",
                 .alt_text = ""});
  document.blocks.push_back(paragraph);

  const std::string html = RenderHtmlDocument(document, "test");
  EXPECT_EQ(html.find("data:text/html"), std::string::npos);
  EXPECT_EQ(html.find("<a href="), std::string::npos);
  EXPECT_NE(html.find("Data Link"), std::string::npos);
}

TEST(HtmlExporterTest, RejectsVbscriptScheme) {
  Document document;
  Block paragraph;
  paragraph.type = BlockType::kParagraph;
  paragraph.inlines.push_back(InlineNode{.type = InlineType::kLink,
                                         .text = "Click",
                                         .url = "vbscript:msgbox(1)",
                                         .alt_text = ""});
  document.blocks.push_back(paragraph);

  const std::string html = RenderHtmlDocument(document, "test");
  EXPECT_EQ(html.find("vbscript:"), std::string::npos);
  EXPECT_EQ(html.find("<a href="), std::string::npos);
}

TEST(HtmlExporterTest, RejectsJavascriptWithMixedCase) {
  Document document;
  Block paragraph;
  paragraph.type = BlockType::kParagraph;
  paragraph.inlines.push_back(InlineNode{.type = InlineType::kLink,
                                         .text = "Click",
                                         .url = "JaVaScRiPt:alert(1)",
                                         .alt_text = ""});
  document.blocks.push_back(paragraph);

  const std::string html = RenderHtmlDocument(document, "test");
  EXPECT_EQ(html.find("javascript"), std::string::npos);
  EXPECT_EQ(html.find("JaVaScRiPt"), std::string::npos);
}

TEST(HtmlExporterTest, EscapesAttributeValues) {
  Document document;
  Block paragraph;
  paragraph.type = BlockType::kParagraph;
  paragraph.inlines.push_back(InlineNode{.type = InlineType::kLink,
                                         .text = "Link",
                                         .url = "https://example.com?a=1&b=2",
                                         .alt_text = ""});
  document.blocks.push_back(paragraph);

  const std::string html = RenderHtmlDocument(document, "test");
  EXPECT_NE(html.find("&amp;"), std::string::npos);
}

TEST(HtmlExporterTest, UsesSlugAnchorsForHeadings) {
  Document document;

  Block heading;
  heading.type = BlockType::kHeading;
  heading.level = 2;
  heading.inlines.push_back(InlineNode{.type = InlineType::kText,
                                       .text = "Core Startup",
                                       .url = "",
                                       .alt_text = ""});
  document.blocks.push_back(heading);

  Block paragraph;
  paragraph.type = BlockType::kParagraph;
  paragraph.inlines.push_back(InlineNode{.type = InlineType::kLink,
                                         .text = "Go",
                                         .url = "#core-startup",
                                         .alt_text = ""});
  document.blocks.push_back(paragraph);

  const std::string html = RenderHtmlDocument(document, "anchors");
  EXPECT_NE(html.find("<h2 id=\"core-startup\">Core Startup</h2>"),
            std::string::npos);
  EXPECT_NE(html.find("<a href=\"#core-startup\">Go</a>"), std::string::npos);
}

TEST(HtmlExporterTest, DeDuplicatesDuplicateHeadingAnchors) {
  Document document;

  Block first;
  first.type = BlockType::kHeading;
  first.level = 2;
  first.inlines.push_back(InlineNode{.type = InlineType::kText,
                                     .text = "Notes",
                                     .url = "",
                                     .alt_text = ""});
  document.blocks.push_back(first);

  Block second = first;
  document.blocks.push_back(second);

  const std::string html = RenderHtmlDocument(document, "dedupe");
  EXPECT_NE(html.find("<h2 id=\"notes\">Notes</h2>"), std::string::npos);
  EXPECT_NE(html.find("<h2 id=\"notes-1\">Notes</h2>"), std::string::npos);
}

}  // namespace
}  // namespace markdown
