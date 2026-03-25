#ifndef MARKDOWN_LOCALE_FILE_PARSER_H_
#define MARKDOWN_LOCALE_FILE_PARSER_H_

#include <map>
#include <string>

namespace markdown {

using LocaleValueMap = std::map<std::string, std::string>;

// Parses locale text formatted as "key=value" lines.
// Keys are normalized to lowercase ASCII. Empty lines and '#' comments are
// ignored. Values support \n, \t, and \\ escaping.
LocaleValueMap ParseLocaleKeyValueText(const std::string& content);

}  // namespace markdown

#endif  // MARKDOWN_LOCALE_FILE_PARSER_H_
