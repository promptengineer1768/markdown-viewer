#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/dirdlg.h>
#include <wx/dnd.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/filesys.h>
#include <wx/frame.h>
#include <wx/html/htmlwin.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <wx/wx.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include "markdown/file_io.h"
#include "markdown/html_exporter.h"
#include "markdown/markdown_parser.h"
#include "markdown/path_utils.h"
#include "markdown/ui_strings.h"

namespace markdown {
namespace {

constexpr int kMenuFileExport = wxID_HIGHEST + 1;
constexpr int kMenuThemeLight = kMenuFileExport + 1;
constexpr int kMenuThemeDark = kMenuThemeLight + 1;
constexpr int kMenuCopyLink = kMenuThemeDark + 1;

enum class ThemeChoice {
  kLight,
  kDark,
};

std::filesystem::path PathFromWxString(const wxString& value) {
#ifdef _WIN32
  return std::filesystem::path(value.ToStdWstring());
#else
  const wxScopedCharBuffer utf8 = value.utf8_str();
  std::string utf8_str(utf8.data(), utf8.length());
  const auto* u8_data = reinterpret_cast<const char8_t*>(utf8_str.data());
  std::u8string u8_str(u8_data, u8_data + utf8_str.size());
  return std::filesystem::path(u8_str);
#endif
}

wxString WxStringFromUtf8(const std::string& value) {
  return wxString::FromUTF8(value.c_str());
}

wxString WxStringFromPath(const std::filesystem::path& path) {
#ifdef _WIN32
  return wxString(path.wstring());
#else
  auto u8_path = path.u8string();
  return WxStringFromUtf8(std::string(u8_path.begin(), u8_path.end()));
#endif
}

std::string Utf8FromWxString(const wxString& value) {
  const wxScopedCharBuffer utf8 = value.utf8_str();
  return std::string(utf8.data(), utf8.length());
}

wxString BuildWildcardFilter(
    const std::vector<FileDialogFilterEntry>& filters) {
  wxString wildcard;
  for (const auto& filter : filters) {
    if (!wildcard.empty()) {
      wildcard += "|";
    }
    wildcard += wxString(filter.label);
    wildcard += "|";
    wildcard += wxString(filter.pattern);
  }
  return wildcard;
}

#ifndef __WXMSW__
std::string ToLowerAscii(std::string value) {
  std::ranges::transform(value, value.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(static_cast<int>(ch)));
  });
  return value;
}

bool ContainsDarkToken(const std::string& value) {
  const std::string lower_value = ToLowerAscii(value);
  return lower_value.find("dark") != std::string::npos ||
         lower_value.find("prefer-dark") != std::string::npos;
}

std::optional<std::string> ExecuteShellCommand(const char* command) {
  FILE* pipe = popen(command, "r");
  if (pipe == nullptr) {
    return std::nullopt;
  }
  std::array<char, 512> buffer = {};
  std::string output;
  while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) !=
         nullptr) {
    output.append(buffer.data());
  }
  const int status = pclose(pipe);
  if (status != 0) {
    return std::nullopt;
  }
  return output;
}

bool DetectGnomeThemeDark() {
  const std::optional<std::string> color_scheme = ExecuteShellCommand(
      "gsettings get org.gnome.desktop.interface color-scheme 2>/dev/null");
  if (color_scheme.has_value() && ContainsDarkToken(*color_scheme)) {
    return true;
  }
  const std::optional<std::string> gtk_theme = ExecuteShellCommand(
      "gsettings get org.gnome.desktop.interface gtk-theme 2>/dev/null");
  return gtk_theme.has_value() && ContainsDarkToken(*gtk_theme);
}

bool DetectKdeThemeDark() {
  const char* home = std::getenv("HOME");
  if (home == nullptr || home[0] == '\0') {
    return false;
  }
  const std::filesystem::path kdeglobals =
      std::filesystem::path(home) / ".config" / "kdeglobals";
  std::ifstream input(kdeglobals);
  if (!input.is_open()) {
    return false;
  }
  std::string line;
  while (std::getline(input, line)) {
    if (line.rfind("ColorScheme=", 0) == 0 && ContainsDarkToken(line)) {
      return true;
    }
  }
  return false;
}
#endif

std::string ToLowerAsciiCopy(std::string value) {
  std::ranges::transform(value, value.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(static_cast<int>(ch)));
  });
  return value;
}

ThemeChoice DetectPreferredTheme() {
#if wxCHECK_VERSION(3, 1, 6)
  const wxSystemAppearance appearance = wxSystemSettings::GetAppearance();
  if (appearance.IsDark()) {
    return ThemeChoice::kDark;
  }
  if (appearance.IsUsingDarkBackground()) {
    return ThemeChoice::kDark;
  }
#endif

#ifndef __WXMSW__
  const char* desktop = std::getenv("XDG_CURRENT_DESKTOP");
  const std::string desktop_value =
      desktop == nullptr ? "" : ToLowerAscii(std::string(desktop));
  if (desktop_value.find("gnome") != std::string::npos ||
      desktop_value.find("ubuntu") != std::string::npos ||
      desktop_value.find("unity") != std::string::npos) {
    return DetectGnomeThemeDark() ? ThemeChoice::kDark : ThemeChoice::kLight;
  }
  if (desktop_value.find("kde") != std::string::npos ||
      desktop_value.find("plasma") != std::string::npos ||
      std::getenv("KDE_FULL_SESSION") != nullptr) {
    return DetectKdeThemeDark() ? ThemeChoice::kDark : ThemeChoice::kLight;
  }

  if (DetectGnomeThemeDark() || DetectKdeThemeDark()) {
    return ThemeChoice::kDark;
  }
#endif

  return ThemeChoice::kLight;
}

class MarkdownFrame;
class PreviewHtmlWindow;

class MarkdownDropTarget : public wxFileDropTarget {
 public:
  explicit MarkdownDropTarget(MarkdownFrame* frame) : frame_(frame) {}
  bool OnDropFiles(wxCoord x, wxCoord y,
                   const wxArrayString& filenames) override;

 private:
  MarkdownFrame* frame_;
};

class PreviewHtmlWindow : public wxHtmlWindow {
 public:
  PreviewHtmlWindow(MarkdownFrame* owner, wxWindow* parent, wxWindowID id,
                    const wxPoint& pos, const wxSize& size, long style);

 protected:
  void OnLinkClicked(const wxHtmlLinkInfo& link) override;

 private:
  MarkdownFrame* owner_;
};

class MarkdownFrame : public wxFrame {
 public:
  MarkdownFrame();

  bool LoadMarkdownFile(const std::filesystem::path& path);
  void LoadDefaultDocumentIfAvailable();

 private:
  void OnOpen(wxCommandEvent& event);
  void OnExport(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnCopy(wxCommandEvent& event);
  void OnCopyLink(wxCommandEvent& event);
  void OnPreviewLinkClicked(wxHtmlLinkEvent& event);
  void OnPreviewCellHover(wxHtmlCellEvent& event);
  void OnPreviewCellClicked(wxHtmlCellEvent& event);
  void OnThemeLight(wxCommandEvent& event);
  void OnThemeDark(wxCommandEvent& event);
  void OnExit(wxCommandEvent& event);

  void SetTheme(ThemeChoice theme);
  void ApplyThemeColors();
  void RefreshPreview();
  void UpdateTitle();
  void ShowError(const wxString& message);
  bool OpenLinkInSystemBrowser(const wxString& href);
  bool CopyTextToClipboard(const wxString& text);
  static std::string GetLinkFromCellEvent(wxHtmlCellEvent& event);

  friend class PreviewHtmlWindow;

  // Ownership model: wxWidgets parent-child ownership.
  // preview_container_ and preview_ are children of this frame.
  // wxWidgets automatically destroys children when parent is destroyed.
  // No manual cleanup needed - relying on wxWidgets auto-cleanup.
  wxPanel* preview_container_ = nullptr;
  PreviewHtmlWindow* preview_ = nullptr;
  Document document_;
  std::filesystem::path current_path_;
  wxString current_title_;
  std::string hovered_link_url_;
  ThemeChoice theme_ = ThemeChoice::kLight;
};

bool MarkdownDropTarget::OnDropFiles(wxCoord /*x*/, wxCoord /*y*/,
                                     const wxArrayString& filenames) {
  if (filenames.IsEmpty()) {
    return false;
  }
  return frame_->LoadMarkdownFile(PathFromWxString(filenames.Item(0)));
}

PreviewHtmlWindow::PreviewHtmlWindow(MarkdownFrame* owner, wxWindow* parent,
                                     wxWindowID id, const wxPoint& pos,
                                     const wxSize& size, long style)
    : wxHtmlWindow(parent, id, pos, size, style), owner_(owner) {}

void PreviewHtmlWindow::OnLinkClicked(const wxHtmlLinkInfo& link) {
  if (owner_ == nullptr) {
    return;
  }

  const std::string href_utf8 = Utf8FromWxString(link.GetHref());
  const LinkReferenceParts link_parts = SplitLinkReference(href_utf8);
  if (!link_parts.suffix.empty() && link_parts.suffix.at(0) == '#') {
    bool same_document = link_parts.path.empty();
    if (!same_document && !owner_->current_path_.empty()) {
      std::filesystem::path candidate_path;
      const std::string path_lower = ToLowerAsciiCopy(link_parts.path);
      if (path_lower.rfind("file:", 0) == 0) {
        const wxFileName file_name =
            wxFileSystem::URLToFileName(WxStringFromUtf8(link_parts.path));
        if (file_name.IsOk()) {
          candidate_path = PathFromWxString(file_name.GetFullPath());
        }
      } else {
        candidate_path = PathFromWxString(WxStringFromUtf8(link_parts.path));
        if (candidate_path.is_relative()) {
          candidate_path = owner_->current_path_.parent_path() / candidate_path;
        }
      }
      if (!candidate_path.empty()) {
        std::error_code ignored;
        const std::filesystem::path current_normalized =
            std::filesystem::weakly_canonical(owner_->current_path_, ignored);
        const std::filesystem::path candidate_normalized =
            std::filesystem::weakly_canonical(candidate_path, ignored);
        same_document = !current_normalized.empty() &&
                        !candidate_normalized.empty() &&
                        current_normalized == candidate_normalized;
      }
    }

    if (same_document) {
      ScrollToAnchor(WxStringFromUtf8(link_parts.suffix.substr(1)));
      return;
    }
  }

  if (!owner_->OpenLinkInSystemBrowser(link.GetHref())) {
    owner_->ShowError("Unable to open link in the default browser.");
  }
}

MarkdownFrame::MarkdownFrame()
    : wxFrame(nullptr, wxID_ANY,
              wxString(GetUiStrings().default_document_title),
              wxDefaultPosition, wxSize(960, 720)),
      preview_container_(new wxPanel(this, wxID_ANY, wxDefaultPosition,
                                     wxDefaultSize, wxBORDER_SIMPLE)),
      preview_(new PreviewHtmlWindow(this, preview_container_, wxID_ANY,
                                     wxDefaultPosition, wxDefaultSize,
                                     wxHW_SCROLLBAR_AUTO)),
      current_title_(wxString(GetUiStrings().default_document_title)),
      theme_(DetectPreferredTheme()) {
  const UiStrings& strings = GetUiStrings();

  auto* file_menu = new wxMenu();
  file_menu->Append(wxID_OPEN, wxString(strings.open_menu));
  file_menu->Append(kMenuFileExport, wxString(strings.export_menu));
  file_menu->AppendSeparator();
  file_menu->Append(wxID_EXIT, wxString(strings.exit_menu));

  auto* edit_menu = new wxMenu();
  edit_menu->Append(wxID_COPY, wxString(strings.copy_menu));

  auto* theme_menu = new wxMenu();
  theme_menu->AppendRadioItem(kMenuThemeLight,
                              wxString(strings.theme_light_menu));
  theme_menu->AppendRadioItem(kMenuThemeDark,
                              wxString(strings.theme_dark_menu));

  auto* preferences_menu = new wxMenu();
  preferences_menu->AppendSubMenu(theme_menu, wxString(strings.theme_menu));

  auto* help_menu = new wxMenu();
  help_menu->Append(wxID_ABOUT, wxString(strings.about_menu));

  auto* menu_bar = new wxMenuBar();
  menu_bar->Append(file_menu, wxString(strings.file_menu));
  menu_bar->Append(edit_menu, wxString(strings.edit_menu));
  menu_bar->Append(preferences_menu, wxString(strings.preferences_menu));
  menu_bar->Append(help_menu, wxString(strings.help_menu));
  SetMenuBar(menu_bar);

  preview_->SetBorders(0);

  auto* preview_sizer = new wxBoxSizer(wxVERTICAL);
  preview_sizer->Add(preview_, wxSizerFlags(1).Expand().Border(wxALL, 6));
  preview_container_->SetSizer(preview_sizer);

  auto* root_sizer = new wxBoxSizer(wxVERTICAL);
  root_sizer->Add(preview_container_,
                  wxSizerFlags(1).Expand().Border(wxALL, 12));
  SetSizer(root_sizer);

  SetDropTarget(new MarkdownDropTarget(this));

  Bind(wxEVT_MENU, &MarkdownFrame::OnOpen, this, wxID_OPEN);
  Bind(wxEVT_MENU, &MarkdownFrame::OnExport, this, kMenuFileExport);
  Bind(wxEVT_MENU, &MarkdownFrame::OnAbout, this, wxID_ABOUT);
  Bind(wxEVT_MENU, &MarkdownFrame::OnCopy, this, wxID_COPY);
  Bind(wxEVT_MENU, &MarkdownFrame::OnCopyLink, this, kMenuCopyLink);
  Bind(wxEVT_MENU, &MarkdownFrame::OnThemeLight, this, kMenuThemeLight);
  Bind(wxEVT_MENU, &MarkdownFrame::OnThemeDark, this, kMenuThemeDark);
  Bind(wxEVT_MENU, &MarkdownFrame::OnExit, this, wxID_EXIT);
  preview_->Bind(wxEVT_HTML_LINK_CLICKED, &MarkdownFrame::OnPreviewLinkClicked,
                 this);
  preview_->Bind(wxEVT_HTML_CELL_HOVER, &MarkdownFrame::OnPreviewCellHover,
                 this);
  preview_->Bind(wxEVT_HTML_CELL_CLICKED, &MarkdownFrame::OnPreviewCellClicked,
                 this);

  GetMenuBar()->Check(kMenuThemeLight, theme_ == ThemeChoice::kLight);
  GetMenuBar()->Check(kMenuThemeDark, theme_ == ThemeChoice::kDark);
  ApplyThemeColors();
  RefreshPreview();
}

bool MarkdownFrame::LoadMarkdownFile(const std::filesystem::path& path) {
  auto content_opt = ReadFileUtf8(path);
  if (!content_opt) {
    ShowError("Unable to open file for reading.");
    return false;
  }

  document_ = ParseMarkdown(*content_opt);
  current_path_ = path;
  current_title_ = WxStringFromUtf8(GetBaseNameWithoutExtensionUtf8(path));
  if (current_title_.empty()) {
    current_title_ = wxString(GetUiStrings().default_document_title);
  }

  UpdateTitle();
  RefreshPreview();
  return true;
}

void MarkdownFrame::LoadDefaultDocumentIfAvailable() {
  std::vector<std::filesystem::path> candidates;
  candidates.push_back(std::filesystem::current_path() / "README.md");

  const std::filesystem::path executable_path =
      PathFromWxString(wxStandardPaths::Get().GetExecutablePath());
  candidates.push_back(executable_path.parent_path() / "README.md");
  candidates.push_back(executable_path.parent_path().parent_path() /
                       "README.md");

  for (const std::filesystem::path& candidate : candidates) {
    std::error_code ignored;
    if (std::filesystem::exists(candidate, ignored) &&
        std::filesystem::is_regular_file(candidate, ignored)) {
      if (LoadMarkdownFile(candidate)) {
        return;
      }
    }
  }
}

void MarkdownFrame::OnOpen(wxCommandEvent& /*event*/) {
  const UiStrings& strings = GetUiStrings();
  wxFileDialog dialog(this, wxString(strings.open_dialog_title), wxEmptyString,
                      wxEmptyString,
                      BuildWildcardFilter(strings.markdown_filters),
                      wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (dialog.ShowModal() != wxID_OK) {
    return;
  }
  LoadMarkdownFile(PathFromWxString(dialog.GetPath()));
}

void MarkdownFrame::OnExport(wxCommandEvent& /*event*/) {
  if (current_path_.empty()) {
    ShowError(wxString(GetUiStrings().open_before_export_error));
    return;
  }

  const UiStrings& strings = GetUiStrings();
  const std::filesystem::path default_path =
      GetDefaultExportPath(current_path_);
  wxFileDialog dialog(this, wxString(strings.export_dialog_title),
                      WxStringFromPath(default_path.parent_path()),
                      WxStringFromPath(default_path.filename()),
                      BuildWildcardFilter(strings.html_filters),
                      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  if (dialog.ShowModal() != wxID_OK) {
    return;
  }

  std::string error;
  const std::string html =
      RenderHtmlDocument(document_, Utf8FromWxString(current_title_),
                         theme_ == ThemeChoice::kDark);
  if (!WriteFileUtf8(PathFromWxString(dialog.GetPath()), html, &error)) {
    ShowError(WxStringFromUtf8(error));
  }
}

void MarkdownFrame::OnAbout(wxCommandEvent& /*event*/) {
  const UiStrings& strings = GetUiStrings();
  wxString about_text;
  about_text += wxString(strings.app_name);
  about_text += "\n";
  about_text += wxString(strings.app_version_label);
  about_text += " ";
  about_text += wxString(strings.app_version);
  about_text += "\n\n";
  about_text += wxString(strings.app_description);
  about_text += "\n\n";
  about_text += wxString(strings.app_copyright);
  wxMessageBox(about_text, wxString(strings.about_dialog_title),
               wxOK | wxICON_INFORMATION, this);
}

void MarkdownFrame::OnCopy(wxCommandEvent& /*event*/) {
  const wxString selection = preview_->SelectionToText();
  if (!selection.empty()) {
    CopyTextToClipboard(selection);
    return;
  }
  if (!hovered_link_url_.empty()) {
    CopyTextToClipboard(WxStringFromUtf8(hovered_link_url_));
  }
}

void MarkdownFrame::OnCopyLink(wxCommandEvent& /*event*/) {
  if (!hovered_link_url_.empty()) {
    CopyTextToClipboard(WxStringFromUtf8(hovered_link_url_));
  }
}

bool MarkdownFrame::OpenLinkInSystemBrowser(const wxString& href) {
  const std::string href_utf8 = Utf8FromWxString(href);
  const LinkReferenceParts link_parts = SplitLinkReference(href_utf8);
  const std::string link_path_lower = ToLowerAsciiCopy(link_parts.path);

  wxString launch_target = href;
  if (link_path_lower.rfind("file:", 0) == 0) {
    const wxFileName filename =
        wxFileSystem::URLToFileName(WxStringFromUtf8(link_parts.path));
    if (!filename.IsOk()) {
      return false;
    }
    std::filesystem::path target_path =
        PathFromWxString(filename.GetFullPath());
    std::error_code ignored;
    const std::filesystem::path normalized =
        std::filesystem::weakly_canonical(target_path, ignored);
    if (!normalized.empty()) {
      target_path = normalized;
    }
    launch_target = wxFileSystem::FileNameToURL(WxStringFromPath(target_path));
    if (!link_parts.suffix.empty()) {
      launch_target += WxStringFromUtf8(link_parts.suffix);
    }
  } else if (!HasUriScheme(href_utf8)) {
    std::filesystem::path target_path;
    if (link_parts.path.empty()) {
      if (current_path_.empty()) {
        return false;
      }
      target_path = current_path_;
    } else {
      target_path = PathFromWxString(WxStringFromUtf8(link_parts.path));
      if (target_path.is_relative()) {
        const std::filesystem::path base_path =
            current_path_.empty() ? std::filesystem::current_path()
                                  : current_path_.parent_path();
        target_path = base_path / target_path;
      }
    }

    std::error_code ignored;
    const std::filesystem::path normalized =
        std::filesystem::weakly_canonical(target_path, ignored);
    if (!normalized.empty()) {
      target_path = normalized;
    }

    launch_target = wxFileSystem::FileNameToURL(WxStringFromPath(target_path));
    if (!link_parts.suffix.empty()) {
      launch_target += WxStringFromUtf8(link_parts.suffix);
    }
  }
  return wxLaunchDefaultBrowser(launch_target);
}

void MarkdownFrame::OnPreviewLinkClicked(wxHtmlLinkEvent& event) {
  if (!OpenLinkInSystemBrowser(event.GetLinkInfo().GetHref())) {
    ShowError("Unable to open link in the default browser.");
  }
}

void MarkdownFrame::OnPreviewCellHover(wxHtmlCellEvent& event) {
  hovered_link_url_ = GetLinkFromCellEvent(event);
  if (!hovered_link_url_.empty()) {
    preview_->SetToolTip(WxStringFromUtf8(hovered_link_url_));
  } else {
    preview_->UnsetToolTip();
  }
  event.Skip();
}

void MarkdownFrame::OnPreviewCellClicked(wxHtmlCellEvent& event) {
  const wxMouseEvent mouse_event = event.GetMouseEvent();
  if (!mouse_event.RightUp()) {
    event.Skip();
    return;
  }

  wxMenu menu;
  const wxString selection = preview_->SelectionToText();
  if (!selection.empty()) {
    menu.Append(wxID_COPY, wxString(GetUiStrings().copy_menu));
  }
  if (!hovered_link_url_.empty()) {
    menu.Append(kMenuCopyLink, wxString(GetUiStrings().copy_link_menu));
  }
  if (menu.GetMenuItemCount() > 0) {
    PopupMenu(&menu);
    return;
  }
  event.Skip();
}

void MarkdownFrame::OnThemeLight(wxCommandEvent& /*event*/) {
  SetTheme(ThemeChoice::kLight);
}

void MarkdownFrame::OnThemeDark(wxCommandEvent& /*event*/) {
  SetTheme(ThemeChoice::kDark);
}

void MarkdownFrame::OnExit(wxCommandEvent& /*event*/) { Close(true); }

void MarkdownFrame::SetTheme(ThemeChoice theme) {
  if (theme_ == theme) {
    return;
  }
  theme_ = theme;
  ApplyThemeColors();
  RefreshPreview();
}

void MarkdownFrame::ApplyThemeColors() {
  if (theme_ == ThemeChoice::kDark) {
    SetBackgroundColour(wxColour(34, 34, 34));
    preview_container_->SetBackgroundColour(wxColour(54, 54, 54));
    preview_->SetBackgroundColour(wxColour(30, 30, 30));
    preview_->SetForegroundColour(wxColour(235, 235, 235));
  } else {
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
    preview_container_->SetBackgroundColour(wxColour(220, 220, 220));
    preview_->SetBackgroundColour(*wxWHITE);
    preview_->SetForegroundColour(*wxBLACK);
  }
  Refresh();
}

void MarkdownFrame::RefreshPreview() {
  std::string base_url;
  if (!current_path_.empty()) {
    base_url = Utf8FromWxString(wxFileSystem::FileNameToURL(
        WxStringFromPath(current_path_.parent_path() / "")));
    if (!base_url.empty() && base_url.back() != '/' &&
        base_url.back() != '\\') {
      base_url += '/';
    }
  }

  const std::string html =
      RenderHtmlDocument(document_, Utf8FromWxString(current_title_),
                         theme_ == ThemeChoice::kDark, base_url);
  preview_->SetPage(WxStringFromUtf8(html));
}

void MarkdownFrame::UpdateTitle() { SetTitle(current_title_); }

void MarkdownFrame::ShowError(const wxString& message) {
  wxMessageBox(message, wxString(GetUiStrings().app_name), wxOK | wxICON_ERROR,
               this);
}

bool MarkdownFrame::CopyTextToClipboard(const wxString& text) {
  if (!wxTheClipboard->Open()) {
    ShowError("Unable to access the clipboard.");
    return false;
  }
  const bool success = wxTheClipboard->SetData(new wxTextDataObject(text));
  wxTheClipboard->Close();
  if (!success) {
    ShowError("Unable to copy text to the clipboard.");
  }
  return success;
}

std::string MarkdownFrame::GetLinkFromCellEvent(wxHtmlCellEvent& event) {
  wxHtmlCell* cell = event.GetCell();
  if (cell == nullptr) {
    return "";
  }

  wxHtmlLinkInfo* link = cell->GetLink(event.GetPoint().x, event.GetPoint().y);
  if (link == nullptr) {
    link = cell->GetLink();
  }
  if (link == nullptr) {
    return "";
  }
  return Utf8FromWxString(link->GetHref());
}

class MarkdownApp : public wxApp {
 public:
  bool OnInit() override {
    wxInitAllImageHandlers();
    const std::filesystem::path executable_path =
        PathFromWxString(wxStandardPaths::Get().GetExecutablePath());
    const std::filesystem::path locales_root =
        executable_path.parent_path() / "locales";
    std::string loaded_locale;
    std::string load_error;
    InitializeUiStrings(locales_root, "", &loaded_locale, &load_error);

    auto* frame = new MarkdownFrame();
    bool loaded_from_argument = false;
    for (int index = 1; index < argc; ++index) {
      // Bounds are guaranteed by the loop condition (index < argc).
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
      const wxString argument = wxString(argv[index]);
      const std::filesystem::path candidate_path = PathFromWxString(argument);
      std::error_code ignored;
      if (!std::filesystem::exists(candidate_path, ignored) ||
          !std::filesystem::is_regular_file(candidate_path, ignored)) {
        continue;
      }
      loaded_from_argument = frame->LoadMarkdownFile(candidate_path);
      if (loaded_from_argument) {
        break;
      }
    }
    if (!loaded_from_argument) {
      frame->LoadDefaultDocumentIfAvailable();
    }
    frame->Show(true);
    return true;
  }
};

}  // namespace
}  // namespace markdown

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif
wxIMPLEMENT_APP(markdown::MarkdownApp);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
