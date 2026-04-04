#include "markdown/path_utils.h"

#include <gtest/gtest.h>

#include <filesystem>

namespace markdown {
namespace {

TEST(PathUtilsTest, StripsMarkdownExtensionFromTitle) {
  const std::filesystem::path path =
      std::filesystem::path(L"docs") / L"example.md";
  EXPECT_EQ(GetBaseNameWithoutExtension(path), L"example");
}

TEST(PathUtilsTest, BuildsDefaultHtmlExportName) {
  const std::filesystem::path path =
      std::filesystem::path(L"docs") / L"notes.markdown";
  const std::filesystem::path expected =
      std::filesystem::path(L"docs") / L"notes.html";
  EXPECT_EQ(GetDefaultExportPath(path), expected);
}

TEST(PathUtilsTest, RoundTripsUtf8Conversion) {
  const std::string utf8_text = "hello";
  const std::wstring wide = WidenUtf8(utf8_text);
  const std::string back = NarrowUtf8(wide);
  EXPECT_EQ(back, utf8_text);
}

TEST(PathUtilsTest, PreservesUnicodeInPathConversion) {
  const std::wstring unicode_name = L"日本語_файл.md";
  const std::filesystem::path path =
      std::filesystem::path(L"docs") / unicode_name;
  const std::string utf8 = NarrowUtf8(path.wstring());
  const std::wstring roundtrip = WidenUtf8(utf8);
  EXPECT_EQ(roundtrip, path.wstring());
}

TEST(PathUtilsTest, BuildsDefaultExportForRelativeDotPath) {
  const std::filesystem::path input = std::filesystem::path(L".") / L"notes.md";
  const std::filesystem::path expected =
      std::filesystem::path(L".") / L"notes.html";
  EXPECT_EQ(GetDefaultExportPath(input), expected);
}

TEST(PathUtilsTest, DetectsUriSchemeForExternalLinks) {
  EXPECT_TRUE(HasUriScheme("https://example.com/docs"));
  EXPECT_TRUE(HasUriScheme("mailto:test@example.com"));
  EXPECT_FALSE(HasUriScheme("#core-startup"));
  EXPECT_FALSE(HasUriScheme("BASIC_API.md#core-startup"));
  EXPECT_FALSE(HasUriScheme("C:/docs/BASIC_API.md#core-startup"));
  EXPECT_FALSE(HasUriScheme("D:\\docs\\BASIC_API.md#core-startup"));
}

TEST(PathUtilsTest, SplitsFragmentOnlyLink) {
  const LinkReferenceParts parts = SplitLinkReference("#core-startup");
  EXPECT_EQ(parts.path, "");
  EXPECT_EQ(parts.suffix, "#core-startup");
}

TEST(PathUtilsTest, SplitsLocalPathAndAnchorLink) {
  const LinkReferenceParts parts =
      SplitLinkReference("BASIC_API.md#core-startup");
  EXPECT_EQ(parts.path, "BASIC_API.md");
  EXPECT_EQ(parts.suffix, "#core-startup");
}

}  // namespace
}  // namespace markdown
