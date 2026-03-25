# Markdown Viewer Visual Test

This file is meant for visual inspection in the app.

---

## Paragraphs and Line Wrapping

This paragraph is intentionally long so you can verify wrapping behavior across the full preview width. It should wrap naturally with no unexpected blank lines and no clipping near the right edge of the preview pane.

This is a second paragraph to confirm spacing between paragraphs is consistent.

## Headings

### Heading Level 3
#### Heading Level 4
##### Heading Level 5
###### Heading Level 6

## Emphasis and Inline Code

Use *italic*, **bold**, and `inline_code` in one sentence.

## Links

Safe links should render as anchors:
- [OpenAI](https://openai.com)
- [Example Docs](https://example.com/docs)
- [Mail Link](mailto:test@example.com)
- [Project README (file link)](../README.md)

Unsafe links should remain plain text:
- [Bad JS](javascript:alert('xss'))

## Lists

- Short item.
- Long item to test wrapping behavior in bullet lists with enough text to force a line break in most window sizes, ensuring continuation stays in the same bullet with no blank line.
- Markdown preview with headings, paragraphs, lists, quotes, code blocks,
  inline code, emphasis, and links.
- Another item after a wrapped one.

- Mixed content item with *italic* and `inline code`.
- Link item: [Visit site](https://example.org)

## Quote Blocks

> This is a quote block.
> It should keep the quote style and spacing across multiple lines.

## Code Fence

```cpp
#include <iostream>

int main() {
  std::cout << "Hello, Markdown Viewer!" << std::endl;
  return 0;
}
```

## Horizontal Rule Again

---

## Combined Stress Case

- Bullet with **bold**, *italic*, `code`, and a [link](https://example.com/path?x=1&y=2).
- Another very long bullet that should wrap cleanly and remain visually attached to the same bullet marker, without inserting an empty line between wrapped segments, and without narrowing the effective content width unexpectedly.

> Final quote for spacing check.
