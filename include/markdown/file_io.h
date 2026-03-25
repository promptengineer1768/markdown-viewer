#ifndef MARKDOWN_FILE_IO_H_
#define MARKDOWN_FILE_IO_H_

#include <filesystem>
#include <optional>
#include <string>

namespace markdown {

/// \brief Reads the entire contents of a file as UTF-8 text.
///
/// Opens the file in binary mode and reads its contents. If the file
/// begins with a UTF-8 BOM (0xEF 0xBB 0xBF), the BOM is automatically
/// stripped from the returned content.
///
/// \param path Path to the file to read. Can be absolute or relative.
///
/// \return The file contents as UTF-8 text if successful.
///         Returns std::nullopt if the file cannot be opened or read.
///
/// \note This function is suitable for text files. For binary files,
///       use platform-specific APIs. Files are read entirely into memory,
///       so very large files may cause memory issues.
///
/// \performance O(n) where n is the file size. Memory usage is proportional
///              to file size plus a small constant.
///
/// \thread_safety This function is thread-safe. Multiple threads may read
///                different files concurrently. Reading the same file from
///                multiple threads is safe if the underlying filesystem
///                supports concurrent reads (which most do).
///
/// \see WriteFileUtf8() for writing files.
///
/// \example
/// \code
/// auto content = ReadFileUtf8("document.md");
/// if (content) {
///     Document doc = ParseMarkdown(*content);
/// }
/// \endcode
std::optional<std::string> ReadFileUtf8(const std::filesystem::path& path);

/// \brief Writes text content to a file as UTF-8.
///
/// Creates or overwrites the file with the provided content. The file is
/// opened in binary mode, so no line ending conversions are performed.
///
/// \param path Path to the file to write. Parent directories must exist.
/// \param content The UTF-8 text to write.
/// \param error Optional pointer to receive error message on failure.
///              If null, error messages are silently discarded.
///
/// \return true if the file was written successfully, false on error.
///
/// \note This function does NOT write a UTF-8 BOM. If you need a BOM,
///       prepend it to the content before calling this function.
///
/// \performance O(n) where n is the content length.
///
/// \thread_safety This function is thread-safe for different files.
///                Writing to the same file from multiple threads requires
///                external synchronization.
///
/// \see ReadFileUtf8() for reading files.
///
/// \example
/// \code
/// std::string html = RenderHtmlDocument(doc, "output");
/// std::string error;
/// if (!WriteFileUtf8("output.html", html, &error)) {
///     std::cerr << "Error: " << error << std::endl;
/// }
/// \endcode
bool WriteFileUtf8(const std::filesystem::path& path,
                   const std::string& content, std::string* error = nullptr);

}  // namespace markdown

#endif  // MARKDOWN_FILE_IO_H_
