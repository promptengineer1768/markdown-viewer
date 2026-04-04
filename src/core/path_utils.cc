#include "markdown/path_utils.h"

#include <cstddef>
#include <string>

namespace markdown {

bool HasUriScheme(const std::string& href) {
  const size_t scheme_separator = href.find(':');
  const size_t first_delimiter = href.find_first_of("/?#");
  if (scheme_separator == 1 && href.size() > 2 &&
      ((href[2] == '/') || (href[2] == '\\'))) {
    const char drive_letter = href[0];
    if ((drive_letter >= 'A' && drive_letter <= 'Z') ||
        (drive_letter >= 'a' && drive_letter <= 'z')) {
      return false;
    }
  }
  return scheme_separator != std::string::npos &&
         (first_delimiter == std::string::npos ||
          scheme_separator < first_delimiter);
}

LinkReferenceParts SplitLinkReference(const std::string& href) {
  const size_t suffix_start = href.find_first_of("?#");
  if (suffix_start == std::string::npos) {
    return {.path = href, .suffix = ""};
  }
  return {.path = href.substr(0, suffix_start),
          .suffix = href.substr(suffix_start)};
}

std::wstring GetBaseNameWithoutExtension(const std::filesystem::path& path) {
  return path.stem().wstring();
}

std::string GetBaseNameWithoutExtensionUtf8(const std::filesystem::path& path) {
  auto u8_str = path.stem().u8string();
  return std::string(u8_str.begin(), u8_str.end());
}

std::filesystem::path GetDefaultExportPath(
    const std::filesystem::path& input_path) {
  std::filesystem::path output_path = input_path;
  output_path.replace_extension(".html");
  return output_path;
}

std::string NarrowUtf8(const std::wstring& value) {
  if (value.empty()) {
    return {};
  }
  // Use std::filesystem::path as a portable UTF conversion bridge.
  // Construct from wstring, extract as u8string, copy to std::string.
  std::filesystem::path tmp(value);
  auto u8_str = tmp.u8string();
  return std::string(u8_str.begin(), u8_str.end());
}

std::wstring WidenUtf8(const std::string& value) {
  if (value.empty()) {
    return {};
  }
  // Use std::filesystem::path as a portable UTF conversion bridge.
  // Construct from u8string via char8_t reinterpretation, extract as wstring.
  const auto* u8_data = reinterpret_cast<const char8_t*>(value.data());
  std::u8string u8_str(u8_data, u8_data + value.size());
  std::filesystem::path tmp(u8_str);
  return tmp.wstring();
}

}  // namespace markdown
