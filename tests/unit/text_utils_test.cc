#include "markdown/text_utils.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace markdown {
namespace {

TEST(TextUtilsTest, TrimRemovesLeadingWhitespace) {
  EXPECT_EQ(Trim("   hello"), "hello");
  EXPECT_EQ(Trim("\t\thello"), "hello");
  EXPECT_EQ(Trim("\nhello"), "hello");
  EXPECT_EQ(Trim(" \t\nhello"), "hello");
}

TEST(TextUtilsTest, TrimRemovesTrailingWhitespace) {
  EXPECT_EQ(Trim("hello   "), "hello");
  EXPECT_EQ(Trim("hello\t\t"), "hello");
  EXPECT_EQ(Trim("hello\n"), "hello");
  EXPECT_EQ(Trim("hello \t\n"), "hello");
}

TEST(TextUtilsTest, TrimRemovesBothEnds) {
  EXPECT_EQ(Trim("  hello  "), "hello");
  EXPECT_EQ(Trim("\thello\n"), "hello");
  EXPECT_EQ(Trim(" \t hello \n "), "hello");
}

TEST(TextUtilsTest, TrimHandlesEmptyString) {
  EXPECT_EQ(Trim(""), "");
  EXPECT_EQ(Trim("   "), "");
  EXPECT_EQ(Trim("\t\n"), "");
}

TEST(TextUtilsTest, TrimPreservesInternalWhitespace) {
  EXPECT_EQ(Trim("  hello  world  "), "hello  world");
  EXPECT_EQ(Trim("\thello\tworld\n"), "hello\tworld");
}

TEST(TextUtilsTest, TrimLeftRemovesOnlyLeading) {
  EXPECT_EQ(TrimLeft("  hello  "), "hello  ");
  EXPECT_EQ(TrimLeft("\thello\n"), "hello\n");
  EXPECT_EQ(TrimLeft(""), "");
  EXPECT_EQ(TrimLeft("   "), "");
}

TEST(TextUtilsTest, SplitLinesOnNewlines) {
  const std::string input = "line1\nline2\nline3";
  const std::vector<std::string> result = SplitLines(input);

  ASSERT_EQ(result.size(), 3U);
  EXPECT_EQ(result[0], "line1");
  EXPECT_EQ(result[1], "line2");
  EXPECT_EQ(result[2], "line3");
}

TEST(TextUtilsTest, SplitLinesHandlesCarriageReturns) {
  const std::string input = "line1\r\nline2\rline3\n";
  const std::vector<std::string> result = SplitLines(input);

  ASSERT_EQ(result.size(), 4U);
  EXPECT_EQ(result[0], "line1");
  EXPECT_EQ(result[1], "line2");
  EXPECT_EQ(result[2], "line3");
  EXPECT_EQ(result[3], "");
}

TEST(TextUtilsTest, SplitLinesHandlesEmptyString) {
  const std::vector<std::string> result = SplitLines("");
  ASSERT_EQ(result.size(), 1U);
  EXPECT_EQ(result[0], "");
}

TEST(TextUtilsTest, SplitLinesHandlesNoNewlines) {
  const std::vector<std::string> result = SplitLines("single line");
  ASSERT_EQ(result.size(), 1U);
  EXPECT_EQ(result[0], "single line");
}

TEST(TextUtilsTest, SplitLinesHandlesConsecutiveNewlines) {
  const std::string input = "a\n\nb";
  const std::vector<std::string> result = SplitLines(input);

  ASSERT_EQ(result.size(), 3U);
  EXPECT_EQ(result[0], "a");
  EXPECT_EQ(result[1], "");
  EXPECT_EQ(result[2], "b");
}

TEST(TextUtilsTest, StartsWithMatchesPrefix) {
  EXPECT_TRUE(StartsWith("hello world", "hello"));
  EXPECT_TRUE(StartsWith("hello world", "h"));
  EXPECT_TRUE(StartsWith("hello world", "hello world"));
}

TEST(TextUtilsTest, StartsWithRejectsNonPrefix) {
  EXPECT_FALSE(StartsWith("hello world", "world"));
  EXPECT_FALSE(StartsWith("hello world", "Hello"));
  EXPECT_FALSE(StartsWith("hello", "hello world"));
}

TEST(TextUtilsTest, StartsWithHandlesEmptyPrefix) {
  EXPECT_TRUE(StartsWith("hello", ""));
  EXPECT_TRUE(StartsWith("", ""));
}

TEST(TextUtilsTest, IsBlankDetectsWhitespaceOnly) {
  EXPECT_TRUE(IsBlank(""));
  EXPECT_TRUE(IsBlank("   "));
  EXPECT_TRUE(IsBlank("\t\n\r"));
  EXPECT_TRUE(IsBlank(" \t \n "));
}

TEST(TextUtilsTest, IsBlankRejectsNonWhitespace) {
  EXPECT_FALSE(IsBlank("hello"));
  EXPECT_FALSE(IsBlank("  hello  "));
  EXPECT_FALSE(IsBlank(" a "));
}

TEST(TextUtilsTest, ToLowerAsciiConvertsUppercase) {
  EXPECT_EQ(ToLowerAscii("HELLO"), "hello");
  EXPECT_EQ(ToLowerAscii("Hello World"), "hello world");
  EXPECT_EQ(ToLowerAscii("ABC123"), "abc123");
}

TEST(TextUtilsTest, ToLowerAsciiPreservesLowercase) {
  EXPECT_EQ(ToLowerAscii("hello"), "hello");
  EXPECT_EQ(ToLowerAscii(""), "");
}

TEST(TextUtilsTest, ToLowerAsciiHandlesNonAlpha) {
  EXPECT_EQ(ToLowerAscii("123!@#"), "123!@#");
  EXPECT_EQ(ToLowerAscii("A-B-C"), "a-b-c");
}

}  // namespace
}  // namespace markdown
