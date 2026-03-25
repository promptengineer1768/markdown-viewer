#include "markdown/text_utils.h"

#include <algorithm>
#include <cctype>
#include <ranges>

namespace markdown {

std::string Trim(std::string value) {
  value.erase(value.begin(), std::ranges::find_if(value, [](unsigned char ch) {
                return !std::isspace(ch);
              }));
  value.erase(
      std::ranges::find_if(std::ranges::reverse_view(value),
                           [](unsigned char ch) { return !std::isspace(ch); })
          .base(),
      value.end());
  return value;
}

std::string TrimLeft(std::string value) {
  value.erase(value.begin(), std::ranges::find_if(value, [](unsigned char ch) {
                return !std::isspace(ch);
              }));
  return value;
}

std::string ToLowerAscii(std::string value) {
  std::ranges::transform(value, value.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return value;
}

std::vector<std::string> SplitLines(const std::string& input) {
  std::vector<std::string> lines;
  std::string current;
  for (size_t i = 0; i < input.size(); ++i) {
    const char ch = input.at(i);
    if (ch == '\r') {
      lines.push_back(current);
      current.clear();
      if (i + 1 < input.size() && input.at(i + 1) == '\n') {
        ++i;
      }
      continue;
    }
    if (ch == '\n') {
      lines.push_back(current);
      current.clear();
    } else {
      current.push_back(ch);
    }
  }
  lines.push_back(current);
  return lines;
}

bool StartsWith(const std::string& value, const std::string& prefix) {
  return value.starts_with(prefix);
}

bool IsBlank(const std::string& value) { return Trim(value).empty(); }

}  // namespace markdown
