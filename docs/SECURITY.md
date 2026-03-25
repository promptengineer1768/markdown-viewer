# Security Considerations

## Overview

This document outlines security considerations for the Markdown Viewer application, particularly around HTML export functionality.

## HTML Escaping

### Characters Escaped

The `EscapeHtml` and `EscapeHtmlAttribute` functions handle the following:

| Character | Entity | Context |
|-----------|--------|---------|
| `&` | `&amp;` | Must be first to prevent double-encoding |
| `<` | `&lt;` | Prevents tag injection |
| `>` | `&gt;` | Prevents tag injection |
| `"` | `&quot;` | Prevents attribute breakout |
| `'` | `&#x27;` | Prevents single-quoted attribute breakout |
| `/` | `&#x2F;` | Prevents `</script>` attacks |

### Control Characters

- Control characters (0x00-0x1F) are stripped from output
- Exceptions: Tab (0x09), Newline (0x0A), Carriage Return (0x0D) are preserved in content
- In attributes, only Tab is preserved (newlines are stripped)

## URL Validation

### Allowed Schemes

Only the following URL schemes are permitted in links and images:

- `http://` - Standard HTTP links
- `https://` - Secure HTTP links
- `mailto:` - Email links
- `ftp://` - FTP links
- Relative URLs (no scheme) - Internal references

### Blocked Schemes

The following schemes are **explicitly blocked**:

- `javascript:` - XSS attacks
- `vbscript:` - VBScript attacks (IE)
- `data:` - Base64-encoded content attacks
- `file:` - Local file access
- Any other scheme not in the allowlist

### URL Validation Process

1. Trim whitespace
2. Check for control characters (< 0x20 except tab)
3. Extract scheme (text before first `:`)
4. If no scheme or scheme appears after `/`, `?`, or `#` - treat as relative (allowed)
5. Compare scheme against allowlist (case-insensitive)

## Attack Vectors Addressed

### XSS via HTML Injection
- All user content is HTML-escaped before rendering
- No raw HTML pass-through

### XSS via JavaScript URLs
- `javascript:` scheme is blocked
- Case-insensitive matching (`JAVASCRIPT:`, `JaVaScRiPt:`)

### XSS via Data URIs
- `data:` scheme is blocked
- Prevents base64-encoded script injection

### XSS via Event Handlers
- Control characters stripped
- Quotes escaped in attributes

### XSS via SVG/MathML
- `<` and `>` are escaped, preventing any tag injection

## Testing

Security tests are located in `tests/unit/html_exporter_test.cc`:

- `RejectsUnsafeJavascriptLinks` - Tests `javascript:` blocking
- `RejectsDataUriScheme` - Tests `data:` blocking
- `RejectsVbscriptScheme` - Tests `vbscript:` blocking
- `RejectsJavascriptWithMixedCase` - Tests case-insensitive blocking
- `EscapesSingleQuote` - Tests `'` escaping
- `EscapesForwardSlash` - Tests `/` escaping
- `RemovesControlCharacters` - Tests control character stripping
- `EscapesAttributeValues` - Tests attribute escaping

## Limitations

1. **No Content Security Policy**: Exported HTML does not include CSP headers. Users viewing exported HTML should serve it with appropriate headers.

2. **No URL Content Validation**: Only the scheme is validated. The URL content itself is not checked for malicious payloads.

3. **No Redirect Validation**: Redirects through URL shorteners or meta refreshes are not detected.

## Recommendations for Users

1. Serve exported HTML with Content-Security-Policy headers
2. Use HTTPS for hosting exported content
3. Sanitize markdown source if accepting from untrusted users
4. Consider using a dedicated sanitizer library for production use
