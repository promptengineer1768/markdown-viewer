#ifndef MARKDOWN_DOCUMENT_H_
#define MARKDOWN_DOCUMENT_H_

#include <cstdint>
#include <string>
#include <vector>

namespace markdown {

/// \brief Inline content types within a block.
///
/// These represent the various inline formatting elements that can appear
/// within paragraphs, headings, and other block-level elements.
enum class InlineType : std::uint8_t {
  kText,           ///< Plain text content
  kEmphasis,       ///< Italic text (*text* or _text_)
  kStrong,         ///< Bold text (**text** or __text__)
  kCode,           ///< Inline code (`code`)
  kLink,           ///< Hyperlink [text](url)
  kImage,          ///< Image ![alt](url)
  kStrikethrough,  ///< Strikethrough ~~text~~
};

/// \brief A single inline content node.
///
/// Represents one piece of inline formatting. The interpretation of fields
/// depends on the type:
/// - kText: Only `text` is used
/// - kEmphasis, kStrong, kCode, kStrikethrough: Only `text` is used
/// - kLink: `text` is the link text, `url` is the destination
/// - kImage: `alt_text` is the alt text, `url` is the source
///
/// \note This struct uses default member initialization for convenience.
struct InlineNode {
  InlineType type = InlineType::kText;  ///< Type of inline content
  std::string text;                     ///< Text content or link text
  std::string url;                      ///< URL for links and images
  std::string alt_text;                 ///< Alt text for images
};

/// \brief Block-level content types.
///
/// These represent the top-level structural elements of a Markdown document.
/// Each block is rendered as a separate HTML element.
enum class BlockType : std::uint8_t {
  kParagraph,        ///< Plain paragraph of text
  kHeading,          ///< Heading (h1-h6)
  kListItem,         ///< Unordered list item
  kOrderedList,      ///< Ordered list item
  kTaskListItem,     ///< Task list item with checkbox
  kCodeBlock,        ///< Fenced code block
  kQuote,            ///< Blockquote
  kHorizontalRule,   ///< Horizontal rule (---)
  kTable,            ///< Table with header and rows
  kDefinitionList,   ///< Definition list (extension)
  kFootnote,         ///< Footnote definition
  kTableOfContents,  ///< [TOC] marker
};

/// \brief Table column alignment options.
enum class ColumnAlignment : std::uint8_t {
  kNone,    ///< No alignment specified (left by default)
  kLeft,    ///< Left-aligned (:---)
  kCenter,  ///< Center-aligned (:---:)
  kRight,   ///< Right-aligned (---:)
};

/// \brief A single block-level element in the document.
///
/// Represents one block of content. The interpretation of fields depends on
/// the block type. See the documentation for each field for details.
///
/// \note This struct uses default member initialization. All fields are
///       optional and only relevant for specific block types.
struct Block {
  BlockType type = BlockType::kParagraph;  ///< Type of block
  int level = 0;        ///< Heading level (1-6) or ordered list start value
  int indentation = 0;  ///< Nesting depth for lists (0-based)
  bool task_checked = false;  ///< Checkbox state for task list items
  std::string language;       ///< Language identifier for fenced code blocks
  std::vector<InlineNode> inlines;  ///< Inline content for most block types
  std::vector<std::string> lines;   ///< Raw lines for code blocks

  /// \brief Table column alignments.
  ///
  /// Specifies the alignment for each column in a table block.
  /// The vector has one entry per column.
  std::vector<ColumnAlignment> alignments;

  /// \brief Table cell content.
  ///
  /// Three-dimensional vector: [row][column][inline_nodes].
  /// Row 0 contains header cells. Subsequent rows contain data cells.
  /// Each cell is a vector of InlineNode for rich content.
  std::vector<std::vector<std::vector<InlineNode>>> table_cells;
};

/// \brief A parsed Markdown document.
///
/// The root structure representing a complete Markdown document.
/// Contains a flat list of Block structures in document order.
///
/// \note This structure is the output of ParseMarkdown() and the input to
///       RenderHtmlDocument(). It is designed to be serializable and
///       does not contain any cyclic references.
///
/// \thread_safety This structure is safe for concurrent read access.
///                Writing is not thread-safe; external synchronization
///                required.
struct Document {
  std::vector<Block> blocks;  ///< Block-level elements in document order
};

}  // namespace markdown

#endif  // MARKDOWN_DOCUMENT_H_
