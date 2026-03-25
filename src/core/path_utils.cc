#include "markdown/path_utils.h"

#include <cstddef>
#include <string>

namespace markdown {

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
