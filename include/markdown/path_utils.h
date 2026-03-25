#ifndef MARKDOWN_PATH_UTILS_H_
#define MARKDOWN_PATH_UTILS_H_

#include <filesystem>
#include <string>

namespace markdown {

/// \brief Extracts the base filename without extension (wide string).
///
/// Returns the last component of the path with the extension removed.
/// This is useful for deriving a title from a filename.
///
/// \param path The file path to process.
///
/// \return The base filename without extension, as a wide string.
///
/// \performance O(n) where n is the path length.
/// \thread_safety Thread-safe for all inputs.
///
/// \example
/// \code
/// // Returns L"document"
/// GetBaseNameWithoutExtension(L"C:/docs/document.md");
/// \endcode
std::wstring GetBaseNameWithoutExtension(const std::filesystem::path& path);

/// \brief Extracts the base filename without extension (UTF-8).
///
/// Same as GetBaseNameWithoutExtension but returns a UTF-8 string.
///
/// \param path The file path to process.
///
/// \return The base filename without extension, as UTF-8.
std::string GetBaseNameWithoutExtensionUtf8(const std::filesystem::path& path);

/// \brief Computes the default export path for an HTML file.
///
/// Given an input Markdown file path, returns the corresponding HTML
/// path with the same base name and .html extension, in the same directory.
///
/// \param input_path Path to the input Markdown file.
///
/// \return The suggested HTML export path.
///
/// \example
/// \code
/// // Returns "C:/docs/document.html"
/// GetDefaultExportPath(L"C:/docs/document.md");
/// \endcode
std::filesystem::path GetDefaultExportPath(
    const std::filesystem::path& input_path);

/// \brief Converts a UTF-8 string to a wide string.
///
/// Converts from UTF-8 encoding to the platform's wide character encoding
/// (UTF-16 on Windows, UTF-32 on most other platforms).
///
/// \param value The UTF-8 string to convert.
///
/// \return The wide string representation.
///
/// \thread_safety Thread-safe for all inputs.
std::wstring WidenUtf8(const std::string& value);

/// \brief Converts a wide string to UTF-8.
///
/// Converts from the platform's wide character encoding to UTF-8.
///
/// \param value The wide string to convert.
///
/// \return The UTF-8 representation.
///
/// \thread_safety Thread-safe for all inputs.
std::string NarrowUtf8(const std::wstring& value);

}  // namespace markdown

#endif  // MARKDOWN_PATH_UTILS_H_
