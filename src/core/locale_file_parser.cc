#include "markdown/locale_file_parser.h"

#include <string>
#include <vector>

#include "markdown/text_utils.h"

namespace markdown {
namespace {

std::string UnescapeValue(std::string value) {
  std::string result;
  result.reserve(value.size());
  for (size_t i = 0; i < value.size(); ++i) {
    const char ch = value.at(i);
    if (ch == '\\' && i + 1 < value.size()) {
      const size_t next_index = i + 1;
      const char next = value.at(next_index);
      if (next == 't') {
        result.push_back('\t');
        ++i;
        continue;
      }
      if (next == 'n') {
        result.push_back('\n');
        ++i;
        continue;
      }
      if (next == '\\') {
        result.push_back('\\');
        ++i;
        continue;
      }
    }
    result.push_back(ch);
  }
  return result;
}

}  // namespace

LocaleValueMap ParseLocaleKeyValueText(const std::string& content) {
  LocaleValueMap values;
  const std::vector<std::string> lines = SplitLines(content);
  for (const std::string& raw_line : lines) {
    const std::string line = Trim(raw_line);
    if (line.empty() || StartsWith(line, "#")) {
      continue;
    }

    const size_t equals = line.find('=');
    if (equals == std::string::npos) {
      continue;
    }

    const std::string key = Trim(line.substr(0, equals));
    const std::string value = Trim(line.substr(equals + 1));
    if (key.empty()) {
      continue;
    }
    values[markdown::ToLowerAscii(key)] = UnescapeValue(value);
  }
  return values;
}

}  // namespace markdown
