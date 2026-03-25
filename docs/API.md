# markdown_core API

`markdown_core` is the reusable C++ library behind the desktop app. It exposes
Markdown parsing, HTML rendering, file/path helpers, and UI localization
utilities.

## Typical Usage

1. Parse input Markdown text into an AST-like `Document`.
2. Render that `Document` into full HTML.

```cpp
#include "markdown/markdown_parser.h"
#include "markdown/html_exporter.h"

const std::string markdown = "# Hello\n\nMarkdown body.";
const markdown::Document doc = markdown::ParseMarkdown(markdown);
const std::string html = markdown::RenderHtmlDocument(doc, "Preview");
```

## Main Headers

- `markdown/markdown_parser.h`: `ParseMarkdown`.
- `markdown/html_exporter.h`: `RenderHtmlDocument`.
- `markdown/path_utils.h`: UTF conversion and export-path helpers.
- `markdown/file_io.h`: UTF-8 file read/write helpers.
- `markdown/ui_strings.h`: localized UI string initialization and retrieval.

## Notes

- Parser complexity is linear with input size (`O(n)` time, `O(n)` space for
  output + intermediate structures).
- Current design is full-document in-memory; streaming support is a future
  enhancement.
