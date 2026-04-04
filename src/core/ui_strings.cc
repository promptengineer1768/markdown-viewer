#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "markdown/ui_strings.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <locale>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include "markdown/file_io.h"
#include "markdown/locale_file_parser.h"
#include "markdown/path_utils.h"
#include "markdown/text_utils.h"

namespace markdown {
namespace {

UiStrings BuildDefaultUiStrings() {
  return UiStrings{
      .app_name = L"Markdown Viewer",
      .app_version = L"1.01",
      .app_version_label = L"Version",
      .app_description = L"Open, preview, and export Markdown documents.",
      .app_copyright = L"(c) 2026 halifaxgeorge",
      .default_document_title = L"Markdown",
      .file_menu = L"&File",
      .edit_menu = L"&Edit",
      .copy_menu = L"&Copy\tCtrl+C",
      .copy_link_menu = L"Copy Link",
      .preferences_menu = L"&Preferences",
      .theme_menu = L"&Theme",
      .theme_light_menu = L"&Light",
      .theme_dark_menu = L"&Dark",
      .help_menu = L"&Help",
      .about_menu = L"&About",
      .open_menu = L"&Open...\tCtrl+O",
      .export_menu = L"&Export As...\tCtrl+E",
      .exit_menu = L"E&xit",
      .open_dialog_title = L"Open Markdown File",
      .export_dialog_title = L"Export HTML File",
      .open_before_export_error = L"Open a Markdown file before exporting.",
      .about_dialog_title = L"About Markdown Viewer",
      .markdown_default_extension = L"md",
      .html_default_extension = L"html",
      .markdown_filters =
          {
              {.label = L"Markdown Files (*.md;*.markdown)",
               .pattern = L"*.md;*.markdown"},
              {.label = L"All Files (*.*)", .pattern = L"*.*"},
          },
      .html_filters = {
          {.label = L"HTML Files (*.html)", .pattern = L"*.html"},
          {.label = L"All Files (*.*)", .pattern = L"*.*"},
      }};
}

UiStrings& MutableUiStrings() {
  static UiStrings ui_strings = BuildDefaultUiStrings();
  return ui_strings;
}

std::string ToLowerAscii(std::string value) {
  std::ranges::transform(value, value.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(static_cast<int>(ch)));
  });
  return value;
}

std::string NormalizeLocaleCode(std::string locale_code) {
  if (locale_code.empty()) {
    return locale_code;
  }
  std::ranges::replace(locale_code, '_', '-');
  locale_code = ToLowerAscii(locale_code);
  if (locale_code.size() >= 5 && locale_code.at(2) == '-') {
    locale_code.at(0) = static_cast<char>(
        std::tolower(static_cast<unsigned char>(locale_code.at(0))));
    locale_code.at(1) = static_cast<char>(
        std::tolower(static_cast<unsigned char>(locale_code.at(1))));
    locale_code.at(3) = static_cast<char>(
        std::toupper(static_cast<unsigned char>(locale_code.at(3))));
    locale_code.at(4) = static_cast<char>(
        std::toupper(static_cast<unsigned char>(locale_code.at(4))));
  }
  return locale_code;
}

std::vector<std::string> SplitString(const std::string& value,
                                     const std::string& delimiter) {
  std::vector<std::string> parts;
  size_t start = 0;
  while (start <= value.size()) {
    const size_t position = value.find(delimiter, start);
    if (position == std::string::npos) {
      parts.push_back(value.substr(start));
      break;
    }
    parts.push_back(value.substr(start, position - start));
    start = position + delimiter.size();
  }
  return parts;
}

std::vector<FileDialogFilterEntry> ParseFilters(
    const std::string& raw_filters) {
  std::vector<FileDialogFilterEntry> filters;
  const std::vector<std::string> entries = SplitString(raw_filters, "||");
  for (const std::string& entry : entries) {
    if (Trim(entry).empty()) {
      continue;
    }
    const size_t separator = entry.find('|');
    if (separator == std::string::npos) {
      continue;
    }
    const std::string label_utf8 = Trim(entry.substr(0, separator));
    const std::string pattern_utf8 = Trim(entry.substr(separator + 1));
    if (label_utf8.empty() || pattern_utf8.empty()) {
      continue;
    }
    filters.push_back(FileDialogFilterEntry{
        .label = WidenUtf8(label_utf8), .pattern = WidenUtf8(pattern_utf8)});
  }
  return filters;
}

bool LoadLanguageFile(const std::filesystem::path& language_file,
                      UiStrings* strings, std::string* error) {
  auto content_opt = ReadFileUtf8(language_file);
  if (!content_opt) {
    if (error != nullptr) {
      *error = "Unable to open file for reading.";
    }
    return false;
  }
  const std::string& content = *content_opt;

  const LocaleValueMap values = ParseLocaleKeyValueText(content);

  auto get_value = [&values](const std::string& key,
                             const std::wstring& fallback) -> std::wstring {
    const auto found = values.find(ToLowerAscii(key));
    if (found == values.end()) {
      return fallback;
    }
    return WidenUtf8(found->second);
  };

  strings->app_name = get_value("app_name", strings->app_name);
  strings->app_version = get_value("app_version", strings->app_version);
  strings->app_version_label =
      get_value("app_version_label", strings->app_version_label);
  strings->app_description =
      get_value("app_description", strings->app_description);
  strings->app_copyright = get_value("app_copyright", strings->app_copyright);
  strings->default_document_title =
      get_value("default_document_title", strings->default_document_title);
  strings->file_menu = get_value("file_menu", strings->file_menu);
  strings->edit_menu = get_value("edit_menu", strings->edit_menu);
  strings->copy_menu = get_value("copy_menu", strings->copy_menu);
  strings->copy_link_menu =
      get_value("copy_link_menu", strings->copy_link_menu);
  strings->preferences_menu =
      get_value("preferences_menu", strings->preferences_menu);
  strings->theme_menu = get_value("theme_menu", strings->theme_menu);
  strings->theme_light_menu =
      get_value("theme_light_menu", strings->theme_light_menu);
  strings->theme_dark_menu =
      get_value("theme_dark_menu", strings->theme_dark_menu);
  strings->help_menu = get_value("help_menu", strings->help_menu);
  strings->about_menu = get_value("about_menu", strings->about_menu);
  strings->open_menu = get_value("open_menu", strings->open_menu);
  strings->export_menu = get_value("export_menu", strings->export_menu);
  strings->exit_menu = get_value("exit_menu", strings->exit_menu);
  strings->open_dialog_title =
      get_value("open_dialog_title", strings->open_dialog_title);
  strings->export_dialog_title =
      get_value("export_dialog_title", strings->export_dialog_title);
  strings->open_before_export_error =
      get_value("open_before_export_error", strings->open_before_export_error);
  strings->about_dialog_title =
      get_value("about_dialog_title", strings->about_dialog_title);
  strings->markdown_default_extension = get_value(
      "markdown_default_extension", strings->markdown_default_extension);
  strings->html_default_extension =
      get_value("html_default_extension", strings->html_default_extension);

  const auto markdown_filters = values.find("markdown_filters");
  if (markdown_filters != values.end()) {
    const std::vector<FileDialogFilterEntry> parsed =
        ParseFilters(markdown_filters->second);
    if (!parsed.empty()) {
      strings->markdown_filters = parsed;
    }
  }
  const auto html_filters = values.find("html_filters");
  if (html_filters != values.end()) {
    const std::vector<FileDialogFilterEntry> parsed =
        ParseFilters(html_filters->second);
    if (!parsed.empty()) {
      strings->html_filters = parsed;
    }
  }

  return true;
}

std::vector<std::string> ListAvailableLocales(
    const std::filesystem::path& locales_root) {
  std::vector<std::string> locales;
  if (!std::filesystem::exists(locales_root)) {
    return locales;
  }
  for (const std::filesystem::directory_entry& entry :
       std::filesystem::directory_iterator(locales_root)) {
    if (!entry.is_directory()) {
      continue;
    }
    auto u8_name = entry.path().filename().u8string();
    locales.emplace_back(u8_name.begin(), u8_name.end());
  }
  std::ranges::sort(locales);
  return locales;
}

std::string ChooseLocale(const std::filesystem::path& locales_root,
                         const std::string& preferred_locale) {
  const std::vector<std::string> available_locales =
      ListAvailableLocales(locales_root);
  if (available_locales.empty()) {
    return "en-US";
  }

  const std::string normalized_preferred =
      NormalizeLocaleCode(preferred_locale);
  if (!normalized_preferred.empty()) {
    for (const std::string& locale : available_locales) {
      if (NormalizeLocaleCode(locale) == normalized_preferred) {
        return locale;
      }
    }

    const size_t dash = normalized_preferred.find('-');
    const std::string language = normalized_preferred.substr(0, dash);
    for (const std::string& locale : available_locales) {
      const std::string normalized_available = NormalizeLocaleCode(locale);
      if (normalized_available.starts_with(language + "-") ||
          normalized_available == language) {
        return locale;
      }
    }
  }

  for (const std::string& locale : available_locales) {
    if (NormalizeLocaleCode(locale) == "en-us") {
      return locale;
    }
  }

  return available_locales.front();
}

}  // namespace

const UiStrings& GetUiStrings() { return MutableUiStrings(); }

bool InitializeUiStrings(const std::filesystem::path& locales_root,
                         const std::string& preferred_locale,
                         std::string* loaded_locale, std::string* error) {
  MutableUiStrings() = BuildDefaultUiStrings();

  const std::string effective_locale =
      preferred_locale.empty() ? DetectSystemLocaleCode() : preferred_locale;
  const std::string selected_locale =
      ChooseLocale(locales_root, effective_locale);
  const std::filesystem::path language_file =
      locales_root / selected_locale / "ui_strings.txt";

  std::string load_error;
  const bool loaded =
      LoadLanguageFile(language_file, &MutableUiStrings(), &load_error);

  if (loaded_locale != nullptr) {
    *loaded_locale = loaded ? selected_locale : "en-US";
  }
  if (!loaded && error != nullptr) {
    *error = load_error;
  }
  return loaded;
}

std::string DetectSystemLocaleCode() {
#ifdef _WIN32
  // Use the C++ standard library locale name on Windows.
  // std::locale("").name() returns something like "English_United States.1252"
  // but we can use GetUserDefaultLocaleName via the C runtime.
  // Simplest portable approach: use environment or default.
  const char* lang = std::getenv("LANG");
  if (lang != nullptr && lang[0] != '\0') {
    std::string locale(lang);
    const size_t dot = locale.find('.');
    if (dot != std::string::npos) {
      locale = locale.substr(0, dot);
    }
    return NormalizeLocaleCode(locale);
  }
  // On Windows without LANG, try to detect via std::locale.
  try {
    std::string name = std::locale("").name();
    // MSVC returns something like "English_United States.1252".
    // We map common ones, or just default to en-US.
    if (name.find("French") != std::string::npos) {
      return "fr-FR";
    }
    if (name.find("Portuguese") != std::string::npos) {
      return "pt-BR";
    }
  } catch (const std::exception&) {
    return "en-US";
  }
  return "en-US";
#else
  const char* lang = std::getenv("LANG");
  if (lang == nullptr || lang[0] == '\0') {
    return "en-US";
  }
  std::string locale(lang);
  const size_t dot = locale.find('.');
  if (dot != std::string::npos) {
    locale = locale.substr(0, dot);
  }
  return NormalizeLocaleCode(locale);
#endif
}

std::vector<wchar_t> BuildFilterBuffer(
    const std::vector<FileDialogFilterEntry>& filters) {
  std::vector<wchar_t> buffer;
  for (const auto& filter : filters) {
    buffer.insert(buffer.end(), filter.label.begin(), filter.label.end());
    buffer.push_back(L'\0');
    buffer.insert(buffer.end(), filter.pattern.begin(), filter.pattern.end());
    buffer.push_back(L'\0');
  }
  buffer.push_back(L'\0');
  return buffer;
}

}  // namespace markdown
