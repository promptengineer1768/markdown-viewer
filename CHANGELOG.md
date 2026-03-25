# Changelog

All notable changes to this project will be documented in this file.

## [1.0.0] - 2026-03-21

### First Release
- **Core Markdown Engine**: High-performance parser with support for headings, paragraphs, lists, and blockquotes
- **Table Support**: Robust support for Markdown tables with alignment controls (`:---`, `---:`, `:---:`)
- **Visual Styles**: Modern typography and responsive layout for document previews
- **Themes**: Full support for Light and Dark modes with smooth transitions
- **Internationalization**: External language packs for `en-US`, `fr-FR`, and `pt-BR`
- **Security**: Built-in XSS sanitization for links and HTML content
- **Export**: One-click export to standard-compliant HTML5
- **Multi-Platform**: Native installers for Windows (MSI, NSIS, ZIP) and Linux (DEB, RPM, AppImage)
- **Fixes**:
    -   Resolved a bug where multi-line list items were incorrectly split into separate blocks, improving rendering of the project README.
    -   Fixed broken relative images in the preview window by implementing absolute path rewriting in the HTML exporter and initializing essential wxWidgets image handlers.
- **Maintenance**:
    -   Completed an exhaustive code quality review for security, internationalization, and adherence to Google's C++, HTML, and Markdown style guides.
    -   Verified cross-compiler and cross-platform compatibility through Clang-Tidy and local build validation.
