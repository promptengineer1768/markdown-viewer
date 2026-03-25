#include "markdown/file_io.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

namespace markdown {
namespace {

class FileIoTest : public ::testing::Test {
 protected:
  std::filesystem::path temp_dir_;

  void SetUp() override {
    temp_dir_ =
        std::filesystem::temp_directory_path() / "markdown_viewer_file_io_test";
    std::error_code ignored;
    std::filesystem::remove_all(temp_dir_, ignored);
    std::filesystem::create_directories(temp_dir_);
  }

  void TearDown() override {
    std::error_code ignored;
    std::filesystem::remove_all(temp_dir_, ignored);
  }

  void WriteRawFile(const std::filesystem::path& path,
                    const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    file.write(content.data(), static_cast<std::streamsize>(content.size()));
  }
};

TEST_F(FileIoTest, ReadsUtf8FileContent) {
  const std::string content = "Hello, World!\nSecond line\n";
  const auto path = temp_dir_ / "test.txt";
  WriteRawFile(path, content);

  auto result = ReadFileUtf8(path);
  EXPECT_EQ(result, std::optional<std::string>(content));
}

TEST_F(FileIoTest, StripsUtf8Bom) {
  const std::string with_bom = std::string("\xEF\xBB\xBF") + "Content with BOM";
  const std::string without_bom = "Content with BOM";
  const auto path = temp_dir_ / "bom.txt";
  WriteRawFile(path, with_bom);

  auto result = ReadFileUtf8(path);
  EXPECT_EQ(result, std::optional<std::string>(without_bom));
}

TEST_F(FileIoTest, HandlesFileWithoutBom) {
  const std::string content = "No BOM here";
  const auto path = temp_dir_ / "no_bom.txt";
  WriteRawFile(path, content);

  auto result = ReadFileUtf8(path);
  EXPECT_EQ(result, std::optional<std::string>(content));
}

TEST_F(FileIoTest, HandlesEmptyFile) {
  const auto path = temp_dir_ / "empty.txt";
  WriteRawFile(path, "");

  auto result = ReadFileUtf8(path);
  EXPECT_EQ(result, std::optional<std::string>(""));
}

TEST_F(FileIoTest, ReturnsNulloptForNonexistentFile) {
  const auto path = temp_dir_ / "nonexistent.txt";
  auto result = ReadFileUtf8(path);
  EXPECT_FALSE(result.has_value());
}

TEST_F(FileIoTest, WritesUtf8Content) {
  const std::string content = "Test content\nWith newlines\n";
  const auto path = temp_dir_ / "write_test.txt";

  std::string error;
  ASSERT_TRUE(WriteFileUtf8(path, content, &error)) << error;

  auto result = ReadFileUtf8(path);
  EXPECT_EQ(result, std::optional<std::string>(content));
}

TEST_F(FileIoTest, OverwritesExistingFile) {
  const auto path = temp_dir_ / "overwrite.txt";
  std::string error;

  ASSERT_TRUE(WriteFileUtf8(path, "Original content", &error)) << error;
  ASSERT_TRUE(WriteFileUtf8(path, "New content", &error)) << error;

  auto result = ReadFileUtf8(path);
  EXPECT_EQ(result, std::optional<std::string>("New content"));
}

TEST_F(FileIoTest, HandlesUnicodeContent) {
  const std::string content = "Hello 世界 🌍 Café";
  const auto path = temp_dir_ / "unicode.txt";

  std::string error;
  ASSERT_TRUE(WriteFileUtf8(path, content, &error)) << error;

  auto result = ReadFileUtf8(path);
  EXPECT_EQ(result, std::optional<std::string>(content));
}

TEST_F(FileIoTest, HandlesNullErrorParameter) {
  const std::string content = "Test";
  const auto path = temp_dir_ / "null_error.txt";
  WriteRawFile(path, content);

  auto result = ReadFileUtf8(path);
  EXPECT_EQ(result, std::optional<std::string>(content));
}

TEST_F(FileIoTest, WritesToDeepPath) {
  const auto deep_path = temp_dir_ / "deep" / "nested" / "dir" / "file.txt";
  std::filesystem::create_directories(deep_path.parent_path());

  std::string error;
  ASSERT_TRUE(WriteFileUtf8(deep_path, "Deep content", &error)) << error;

  auto result = ReadFileUtf8(deep_path);
  EXPECT_EQ(result, std::optional<std::string>("Deep content"));
}

TEST_F(FileIoTest, HandlesBomOnlyFile) {
  const std::string bom_only = "\xEF\xBB\xBF";
  const auto path = temp_dir_ / "bom_only.txt";
  WriteRawFile(path, bom_only);

  auto result = ReadFileUtf8(path);
  EXPECT_EQ(result, std::optional<std::string>(""));
}

TEST_F(FileIoTest, HandlesPartialBom) {
  const std::string partial_bom = "\xEF\xBB";
  const auto path = temp_dir_ / "partial_bom.txt";
  WriteRawFile(path, partial_bom);

  auto result = ReadFileUtf8(path);
  EXPECT_EQ(result, std::optional<std::string>(partial_bom));
}

TEST_F(FileIoTest, HandlesLargeFile) {
  const size_t size = static_cast<size_t>(10) * 1024 * 1024;
  std::string large_content(size, 'x');
  for (size_t i = 0; i < size; i += 1000) {
    large_content.at(i) = '\n';
  }

  const auto path = temp_dir_ / "large.txt";
  std::string error;
  ASSERT_TRUE(WriteFileUtf8(path, large_content, &error)) << error;

  auto result = ReadFileUtf8(path);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value_or("").size(), size);
}

}  // namespace
}  // namespace markdown
