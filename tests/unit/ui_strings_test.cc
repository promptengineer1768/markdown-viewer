#include "markdown/ui_strings.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "markdown/file_io.h"

namespace markdown {
namespace {

std::filesystem::path BuildTempLocalesRoot() {
  const std::filesystem::path root = std::filesystem::temp_directory_path() /
                                     "markdown_viewer_ui_strings_test";
  std::error_code ignored;
  std::filesystem::remove_all(root, ignored);
  std::filesystem::create_directories(root / "en-US");
  std::filesystem::create_directories(root / "fr-FR");
  std::filesystem::create_directories(root / "pt-BR");
  return root;
}

void WriteLanguageFile(const std::filesystem::path& root,
                       const std::string& locale, const std::string& app_name,
                       const std::string& file_menu) {
  std::string error;
  const std::string content =
      "app_name=" + app_name +
      "\n"
      "file_menu=" +
      file_menu +
      "\n"
      "open_menu=&Open...\\tCtrl+O\n"
      "export_menu=&Export As...\\tCtrl+E\n"
      "exit_menu=E&xit\n"
      "open_dialog_title=Open\n"
      "export_dialog_title=Export\n"
      "open_before_export_error=Open file first\n"
      "default_document_title=Markdown\n"
      "markdown_default_extension=md\n"
      "html_default_extension=html\n"
      "markdown_filters=Markdown Files (*.md)|*.md||All Files (*.*)|*.*\n"
      "html_filters=HTML Files (*.html)|*.html||All Files (*.*)|*.*\n";
  ASSERT_TRUE(WriteFileUtf8(root / locale / "ui_strings.txt", content, &error))
      << error;
}

TEST(UiStringsTest, LoadsExactLocaleFile) {
  const std::filesystem::path locales_root = BuildTempLocalesRoot();
  WriteLanguageFile(locales_root, "fr-FR", "Visionneuse Markdown", "&Fichier");
  WriteLanguageFile(locales_root, "en-US", "Markdown Viewer", "&File");

  std::string loaded_locale;
  std::string error;
  ASSERT_TRUE(
      InitializeUiStrings(locales_root, "fr-FR", &loaded_locale, &error))
      << error;
  EXPECT_EQ(loaded_locale, "fr-FR");
  EXPECT_EQ(GetUiStrings().app_name, L"Visionneuse Markdown");
  EXPECT_EQ(GetUiStrings().file_menu, L"&Fichier");
}

TEST(UiStringsTest, FallsBackToMatchingLanguageLocale) {
  const std::filesystem::path locales_root = BuildTempLocalesRoot();
  WriteLanguageFile(locales_root, "fr-FR", "Visionneuse Markdown", "&Fichier");
  WriteLanguageFile(locales_root, "en-US", "Markdown Viewer", "&File");

  std::string loaded_locale;
  std::string error;
  ASSERT_TRUE(
      InitializeUiStrings(locales_root, "fr-CA", &loaded_locale, &error))
      << error;
  EXPECT_EQ(loaded_locale, "fr-FR");
  EXPECT_EQ(GetUiStrings().app_name, L"Visionneuse Markdown");
}

TEST(UiStringsTest, FallsBackToEnglishWhenLocaleMissing) {
  const std::filesystem::path locales_root = BuildTempLocalesRoot();
  WriteLanguageFile(locales_root, "en-US", "Markdown Viewer", "&File");
  WriteLanguageFile(locales_root, "pt-BR", "Visualizador Markdown", "&Arquivo");

  std::string loaded_locale;
  std::string error;
  ASSERT_TRUE(
      InitializeUiStrings(locales_root, "de-DE", &loaded_locale, &error))
      << error;
  EXPECT_EQ(loaded_locale, "en-US");
  EXPECT_EQ(GetUiStrings().app_name, L"Markdown Viewer");
  EXPECT_EQ(GetUiStrings().file_menu, L"&File");
}

}  // namespace
}  // namespace markdown
