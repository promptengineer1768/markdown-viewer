#pragma once

#ifdef MARKDOWN_VIEWER_ENABLE_GETTEXT
#include <libintl.h>

#include <clocale>
#else
#define _(msgid) (msgid)
#define _N(msgid, msgid_plural, n) ((n) == 1 ? (msgid) : (msgid_plural))
#endif

namespace markdown {

#ifdef MARKDOWN_VIEWER_ENABLE_GETTEXT
inline void InitI18N(const char* domain, const char* locale_dir) {
  setlocale(LC_ALL, "");
  bindtextdomain(domain, locale_dir);
  bind_textdomain_codeset(domain, "UTF-8");
  textdomain(domain);
}
#else
inline void InitI18N(const char* /*domain*/, const char* /*locale_dir*/) {}
#endif

}  // namespace markdown
