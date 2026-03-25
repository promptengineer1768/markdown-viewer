#include <gtest/gtest.h>

#include <string>

#include "markdown/html_exporter.h"
#include "markdown/markdown_parser.h"

namespace markdown {
namespace {

TEST(IntegrationTest, ParseAndRenderHeading) {
  const std::string markdown = "# Hello World";
  const Document document = ParseMarkdown(markdown);
  const std::string html = RenderHtmlDocument(document, "Test");

  EXPECT_NE(html.find("<h1"), std::string::npos);
  EXPECT_NE(html.find("Hello World</h1>"), std::string::npos);
}

TEST(IntegrationTest, ParseAndRenderParagraph) {
  const std::string markdown = "This is a paragraph.";
  const Document document = ParseMarkdown(markdown);
  const std::string html = RenderHtmlDocument(document, "Test");

  EXPECT_NE(html.find("<p>"), std::string::npos);
  EXPECT_NE(html.find("This is a paragraph.</p>"), std::string::npos);
}

TEST(IntegrationTest, ParseAndRenderList) {
  const std::string markdown = "- Item 1\n- Item 2\n- Item 3";
  const Document document = ParseMarkdown(markdown);
  const std::string html = RenderHtmlDocument(document, "Test");

  EXPECT_NE(html.find("<ul>"), std::string::npos);
  EXPECT_NE(html.find("<li>Item 1</li>"), std::string::npos);
  EXPECT_NE(html.find("<li>Item 2</li>"), std::string::npos);
  EXPECT_NE(html.find("<li>Item 3</li>"), std::string::npos);
  EXPECT_NE(html.find("</ul>"), std::string::npos);
}

TEST(IntegrationTest, ParseAndRenderCodeBlock) {
  const std::string markdown = "```cpp\nint x = 42;\n```";
  const Document document = ParseMarkdown(markdown);
  const std::string html = RenderHtmlDocument(document, "Test");

  EXPECT_NE(html.find("<pre>"), std::string::npos);
  EXPECT_NE(html.find("<code>"), std::string::npos);
  EXPECT_NE(html.find("int x = 42;"), std::string::npos);
  EXPECT_NE(html.find("</code></pre>"), std::string::npos);
}

TEST(IntegrationTest, ParseAndRenderTable) {
  const std::string markdown =
      "| Name | Value |\n"
      "|------|-------|\n"
      "| A    | 1     |\n"
      "| B    | 2     |";
  const Document document = ParseMarkdown(markdown);
  const std::string html = RenderHtmlDocument(document, "Test");

  EXPECT_NE(html.find("<table>"), std::string::npos);
  EXPECT_NE(html.find("Name"), std::string::npos);
  EXPECT_NE(html.find("Value"), std::string::npos);
  EXPECT_NE(html.find("</table>"), std::string::npos);
}

TEST(IntegrationTest, ParseAndRenderBlockquote) {
  const std::string markdown = "> This is a quote";
  const Document document = ParseMarkdown(markdown);
  const std::string html = RenderHtmlDocument(document, "Test");

  EXPECT_NE(html.find("<blockquote>"), std::string::npos);
  EXPECT_NE(html.find("This is a quote"), std::string::npos);
  EXPECT_NE(html.find("</blockquote>"), std::string::npos);
}

TEST(IntegrationTest, ParseAndRenderBoldAndItalic) {
  const std::string markdown = "**bold** and *italic*";
  const Document document = ParseMarkdown(markdown);
  const std::string html = RenderHtmlDocument(document, "Test");

  EXPECT_NE(html.find("<strong>bold</strong>"), std::string::npos);
  EXPECT_NE(html.find("<em>italic</em>"), std::string::npos);
}

TEST(IntegrationTest, ParseAndRenderLink) {
  const std::string markdown = "[Click here](https://example.com)";
  const Document document = ParseMarkdown(markdown);
  const std::string html = RenderHtmlDocument(document, "Test");

  EXPECT_NE(html.find("<a href=\"https://example.com\">"), std::string::npos);
  EXPECT_NE(html.find("Click here</a>"), std::string::npos);
}

TEST(IntegrationTest, ParseAndRenderImage) {
  const std::string markdown = "![Alt text](image.png)";
  const Document document = ParseMarkdown(markdown);
  const std::string html = RenderHtmlDocument(document, "Test");

  EXPECT_NE(html.find("<img"), std::string::npos);
  EXPECT_NE(html.find("src=\"image.png\""), std::string::npos);
  EXPECT_NE(html.find("alt=\"Alt text\""), std::string::npos);
}

TEST(IntegrationTest, ParseAndRenderCompleteDocument) {
  const std::string markdown =
      "# Title\n\n"
      "Introduction paragraph.\n\n"
      "## Section\n\n"
      "- List item 1\n"
      "- List item 2\n\n"
      "> Quote\n\n"
      "```\ncode\n```\n";

  const Document document = ParseMarkdown(markdown);
  const std::string html = RenderHtmlDocument(document, "Complete");

  EXPECT_NE(html.find("<h1"), std::string::npos);
  EXPECT_NE(html.find("<h2"), std::string::npos);
  EXPECT_NE(html.find("<ul>"), std::string::npos);
  EXPECT_NE(html.find("<blockquote>"), std::string::npos);
  EXPECT_NE(html.find("<pre>"), std::string::npos);
  EXPECT_NE(html.find("<code>"), std::string::npos);
  EXPECT_NE(html.find("</body>"), std::string::npos);
  EXPECT_NE(html.find("</html>"), std::string::npos);
}

TEST(IntegrationTest, DarkThemeAppliesColors) {
  const std::string markdown = "Test";
  const Document document = ParseMarkdown(markdown);
  const std::string html = RenderHtmlDocument(document, "Dark", true);

  EXPECT_NE(html.find("bgcolor=\"#1e1e1e\""), std::string::npos);
  EXPECT_NE(html.find("text=\"#ededed\""), std::string::npos);
}

TEST(IntegrationTest, LightThemeAppliesColors) {
  const std::string markdown = "Test";
  const Document document = ParseMarkdown(markdown);
  const std::string html = RenderHtmlDocument(document, "Light", false);

  EXPECT_NE(html.find("bgcolor=\"#ffffff\""), std::string::npos);
}

}  // namespace
}  // namespace markdown
