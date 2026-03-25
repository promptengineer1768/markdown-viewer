#ifndef MARKDOWN_MARKDOWN_PARSER_H_
#define MARKDOWN_MARKDOWN_PARSER_H_

#include <string>

#include "markdown/document.h"

namespace markdown {

/// \brief Parses Markdown source text into a Document AST.
///
/// Converts raw Markdown text into a structured Document object that can
/// be rendered to HTML or processed programmatically.
///
/// \param input Raw Markdown text in UTF-8 encoding.
///              May contain any mix of line endings (CR, LF, CRLF).
///
/// \return A Document structure containing the parsed blocks and inline
///         elements. Empty input returns an empty document.
///
/// \note This is not a strict CommonMark parser. It focuses on common
///       Markdown patterns and may not handle all edge cases correctly.
///
/// \performance O(n) single-pass parser where n is the input length.
///              Uses no recursion for block parsing; inline parsing may
///              use recursion for nested structures.
///
/// \limits The parser has the following practical limits:
///         - Maximum heading level: 6 (per Markdown spec)
///         - Maximum list nesting: Unbounded (use with caution)
///         - Maximum table columns: Unbounded
///         - Maximum line length: Limited only by std::string
///         - Maximum document size: Limited only by available memory
///
/// \thread_safety This function is thread-safe. Multiple threads may
///                parse different documents concurrently.
///
/// \see RenderHtmlDocument() for converting the result to HTML.
/// \see Document for the structure of the returned AST.
///
/// \example
/// \code
/// std::string markdown = "# Hello\n\nThis is **bold**.";
/// Document doc = ParseMarkdown(markdown);
/// for (const Block& block : doc.blocks) {
///     // Process each block
/// }
/// \endcode
Document ParseMarkdown(const std::string& input);

}  // namespace markdown

#endif  // MARKDOWN_MARKDOWN_PARSER_H_
