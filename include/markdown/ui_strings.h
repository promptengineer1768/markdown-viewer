#ifndef MARKDOWN_UI_STRINGS_H_
#define MARKDOWN_UI_STRINGS_H_

#include <filesystem>
#include <string>
#include <vector>

namespace markdown {

struct FileDialogFilterEntry {
  std::wstring label;
  std::wstring pattern;
};

struct UiStrings {
  std::wstring app_name;
  std::wstring app_version;
  std::wstring app_version_label;
  std::wstring app_description;
  std::wstring app_copyright;
  std::wstring default_document_title;
  std::wstring file_menu;
  std::wstring edit_menu;
  std::wstring copy_menu;
  std::wstring copy_link_menu;
  std::wstring preferences_menu;
  std::wstring theme_menu;
  std::wstring theme_light_menu;
  std::wstring theme_dark_menu;
  std::wstring help_menu;
  std::wstring about_menu;
  std::wstring open_menu;
  std::wstring export_menu;
  std::wstring exit_menu;
  std::wstring open_dialog_title;
  std::wstring export_dialog_title;
  std::wstring open_before_export_error;
  std::wstring about_dialog_title;
  std::wstring markdown_default_extension;
  std::wstring html_default_extension;
  std::vector<FileDialogFilterEntry> markdown_filters;
  std::vector<FileDialogFilterEntry> html_filters;
};

const UiStrings& GetUiStrings();
bool InitializeUiStrings(const std::filesystem::path& locales_root,
                         const std::string& preferred_locale,
                         std::string* loaded_locale, std::string* error);
std::string DetectSystemLocaleCode();
std::vector<wchar_t> BuildFilterBuffer(
    const std::vector<FileDialogFilterEntry>& filters);

}  // namespace markdown

#endif  // MARKDOWN_UI_STRINGS_H_
