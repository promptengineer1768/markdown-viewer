#ifndef MARKDOWN_TEXT_UTILS_H_
#define MARKDOWN_TEXT_UTILS_H_

#include <string>
#include <vector>

namespace markdown {

/// \brief Removes leading and trailing whitespace from a string.
///
/// Removes all whitespace characters (space, tab, newline, etc.) from
/// both ends of the string. The original string is modified.
///
/// \param value The string to trim. Passed by value for efficiency with
///              move semantics.
///
/// \return The trimmed string.
///
/// \performance O(n) where n is the string length.
/// \thread_safety Thread-safe for all inputs.
std::string Trim(std::string value);

/// \brief Removes leading whitespace from a string.
///
/// Removes all whitespace characters from the beginning of the string.
///
/// \param value The string to trim.
///
/// \return The left-trimmed string.
///
/// \performance O(n) where n is the string length.
/// \thread_safety Thread-safe for all inputs.
std::string TrimLeft(std::string value);

/// \brief Converts a string to lowercase (ASCII only).
///
/// Converts all ASCII uppercase letters (A-Z) to lowercase (a-z).
/// Non-ASCII characters are not modified. This is suitable for
/// case-insensitive comparison of ASCII identifiers like URL schemes.
///
/// \param value The string to convert.
///
/// \return The lowercase string.
///
/// \note This function only handles ASCII. For Unicode case conversion,
///       use a Unicode-aware library.
///
/// \performance O(n) where n is the string length.
/// \thread_safety Thread-safe for all inputs.
std::string ToLowerAscii(std::string value);

/// \brief Splits a string into lines.
///
/// Splits the input at newline characters, returning a vector of lines.
/// Carriage returns are stripped. Empty lines are preserved.
///
/// \param input The string to split.
///
/// \return A vector of lines, one per newline in the input.
///         The last element is the text after the last newline.
///
/// \performance O(n) where n is the input length.
/// \thread_safety Thread-safe for all inputs.
std::vector<std::string> SplitLines(const std::string& input);

/// \brief Checks if a string starts with a given prefix.
///
/// \param value The string to check.
/// \param prefix The prefix to look for.
///
/// \return true if value starts with prefix, false otherwise.
///
/// \performance O(m) where m is the prefix length.
/// \thread_safety Thread-safe for all inputs.
bool StartsWith(const std::string& value, const std::string& prefix);

/// \brief Checks if a string contains only whitespace.
///
/// \param value The string to check.
///
/// \return true if the string is empty or contains only whitespace.
///
/// \performance O(n) where n is the string length.
/// \thread_safety Thread-safe for all inputs.
bool IsBlank(const std::string& value);

}  // namespace markdown

#endif  // MARKDOWN_TEXT_UTILS_H_
