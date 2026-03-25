#include "markdown/locale_file_parser.h"

#include <gtest/gtest.h>

#include <string>

namespace markdown {
namespace {

TEST(LocaleFileParserTest, ParsesSimpleKeyValuePairs) {
  const std::string content = "app_name=Markdown Viewer\nopen_menu=&Open";
  const LocaleValueMap values = ParseLocaleKeyValueText(content);

  ASSERT_EQ(values.size(), 2U);
  EXPECT_EQ(values.at("app_name"), "Markdown Viewer");
  EXPECT_EQ(values.at("open_menu"), "&Open");
}

TEST(LocaleFileParserTest, IgnoresCommentsAndMalformedLines) {
  const std::string content =
      "# comment\n"
      "invalid-line\n"
      "app_name=Viewer\n";
  const LocaleValueMap values = ParseLocaleKeyValueText(content);

  ASSERT_EQ(values.size(), 1U);
  EXPECT_EQ(values.at("app_name"), "Viewer");
}

TEST(LocaleFileParserTest, NormalizesKeysAndUnescapesValues) {
  const std::string content = R"(Open_Menu=Open\tNow\nPath\\Name)";
  const LocaleValueMap values = ParseLocaleKeyValueText(content);

  ASSERT_EQ(values.size(), 1U);
  EXPECT_EQ(values.at("open_menu"), "Open\tNow\nPath\\Name");
}

}  // namespace
}  // namespace markdown
