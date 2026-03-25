# Markdown Viewer

A modern, cross-platform desktop application for viewing and exporting Markdown documents.

| Light Theme | Dark Theme |
| :--- | :--- |
| ![Light Theme](resources/MD_Viewer_Light_Theme.jpg) | ![Dark Theme](resources/MD_Viewer_Dark_Theme.jpg) |

## Features

- **Full Markdown Support**: Headings, paragraphs, blockquotes, tables, ordered and unordered lists, task lists, code blocks, and horizontal rules
- **Advanced Formatting**: Table of Contents (`[TOC]`), footnotes (`[^1]`), reference-style links, and strikethrough text
- **Modern Interface**: Clean, distraction-free reading experience with light and dark theme support
- **Automatic Theme Detection**: Matches your system's color scheme automatically
- **Internationalization**: Available in English, French, and Portuguese
- **HTML Export**: Convert any Markdown document to standalone HTML with one click
- **Cross-Platform**: Works on Windows, macOS, and Linux

## Getting Started

### Opening a File

1. Click **File > Open** or press `Ctrl+O` (`Cmd+O` on macOS)
2. Select a Markdown file (`.md` or `.markdown`)
3. The document renders immediately in the preview pane

### Supported File Types

- `.md` - Markdown files
- `.markdown` - Markdown files

### Navigating Links

Click any link in the document to open it in your default web browser.

## Using the Application

### Theme Selection

Switch between themes via **Preferences > Theme**:

- **Light**: Bright background, dark text
- **Dark**: Dark background, light text (reduces eye strain in low-light environments)

The application automatically detects your system theme on first launch.

### Copying Content

Select text in the preview and use **Edit > Copy** (`Ctrl+C` / `Cmd+C`) to copy to clipboard.

### Exporting to HTML

1. Open a Markdown file
2. Click **File > Export HTML**
3. Choose the destination folder
4. The HTML file preserves all formatting and styling

### Keyboard Shortcuts

| Action | Windows/Linux | macOS |
| :--- | :--- | :--- |
| Open File | `Ctrl+O` | `Cmd+O` |
| Copy | `Ctrl+C` | `Cmd+C` |
| About | `F1` | `Cmd+?` |

## Markdown Syntax Guide

### Basic Formatting

```markdown
# Heading 1
## Heading 2
### Heading 3

**bold text**
*italic text*
~~strikethrough text~~

- Unordered list item
- Another item

1. Ordered list item
2. Another item

- [ ] Task (incomplete)
- [x] Task (completed)

> Blockquote

`inline code`

```
Code block
```
```

### Links and Images

```markdown
[Link text](https://example.com)

![Image alt text](path/to/image.png)

[Reference link][ref]
[ref]: https://example.com
```

### Tables

```markdown
| Column 1 | Column 2 | Column 3 |
| :--- | :---: | ---: |
| Left | Center | Right |
```

### Advanced Features

```markdown
[TOC]

Footnote reference[^1]

[^1]: Footnote text goes here
```

## Troubleshooting

### The application won't start

Ensure all required files are present in the installation directory:
- The executable (`markdown_viewer.exe` on Windows)
- The `bin/` folder with all dependencies
- The `locales/` folder for translations

### Images don't display

- Verify image paths are correct relative to the Markdown file location
- Supported formats: PNG, JPG, GIF, WebP

### Theme doesn't match system

Use **Preferences > Theme** to manually select Light or Dark theme.

## System Requirements

### Windows
- Windows 10 or later
- 64-bit system

### macOS
- macOS 10.15 (Catalina) or later

### Linux
- Ubuntu 20.04 or later, or compatible distribution
- wxWidgets 3.2 or later

## Getting Help

- Report issues: https://github.com/promptengineer1768/markdown-viewer/issues
- View source code: https://github.com/promptengineer1768/markdown-viewer

## License

Markdown Viewer is released under the GNU General Public License v3.0.

```
Markdown Viewer - A modern desktop Markdown viewer
Copyright (C) 2026 George Halifax

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
```
