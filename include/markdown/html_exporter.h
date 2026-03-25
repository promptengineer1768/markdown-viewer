#ifndef MARKDOWN_HTML_EXPORTER_H_
#define MARKDOWN_HTML_EXPORTER_H_

#include <string>

#include "markdown/document.h"

namespace markdown {

/// \brief Renders a Document AST to a complete HTML5 document.
///
/// Converts the parsed Document structure into a self-contained HTML5
/// document with embedded CSS for styling. The output is suitable for
/// viewing in a browser or saving as a file.
///
/// \param document The parsed document to render.
/// \param title The title for the HTML document (appears in browser tab).
/// \param dark_theme If true, renders with dark theme colors and styling.
/// \param base_url Optional base URL for resolving relative links and images.
///                 If provided, relative URLs in the document will be
///                 resolved relative to this URL.
///
/// \return A complete HTML5 document as a UTF-8 encoded string.
///
/// \security All user content is HTML-escaped before rendering to prevent
///           XSS attacks. Links are validated to only allow safe URL schemes
///           (http, https, mailto, ftp). The 'javascript:' and 'data:' schemes
///           are explicitly blocked.
///
/// \performance O(n) where n is the total content size. Memory usage is
///              proportional to output size.
///
/// \thread_safety This function is thread-safe. Multiple threads may
///                render different documents concurrently.
///
/// \see ParseMarkdown() for creating a Document from Markdown text.
/// \see docs/SECURITY.md for details on HTML escaping and URL validation.
///
/// \example
/// \code
/// Document doc = ParseMarkdown("# Hello\n\nWorld");
/// std::string html = RenderHtmlDocument(doc, "My Page", true);
/// // html now contains a complete HTML5 document with dark theme
/// \endcode
std::string RenderHtmlDocument(const Document& document,
                               const std::string& title,
                               bool dark_theme = false,
                               const std::string& base_url = "");

}  // namespace markdown

#endif  // MARKDOWN_HTML_EXPORTER_H_
