#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

// Test for packaging configuration
// These tests verify the installer and package configuration to prevent
// regressions.

class PackagingTest : public ::testing::Test {
 protected:
  std::filesystem::path project_root_;

  void SetUp() override {
    // Find project root by looking for CMakeLists.txt
    auto current = std::filesystem::current_path();
    while (current.has_parent_path()) {
      if (std::filesystem::exists(current / "CMakeLists.txt")) {
        project_root_ = current;
        break;
      }
      current = current.parent_path();
    }
  }

  std::string ReadFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
      return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }
};

TEST_F(PackagingTest, InnoSetupScriptExists) {
  // Verify Inno Setup script exists
  auto inno_script = project_root_ / "installer" / "markdown-viewer.iss";
  EXPECT_TRUE(std::filesystem::exists(inno_script))
      << "Inno Setup script must exist at installer/markdown-viewer.iss";
}

TEST_F(PackagingTest, InnoSetupHasDesktopShortcut) {
  // Verify Inno Setup includes desktop shortcut configuration
  auto inno_script = project_root_ / "installer" / "markdown-viewer.iss";
  std::string content = ReadFile(inno_script);

  EXPECT_FALSE(content.empty()) << "Could not read Inno Setup script";

  // Check for desktop shortcut task
  EXPECT_NE(content.find("[Tasks]"), std::string::npos)
      << "Inno Setup must have [Tasks] section";
  EXPECT_NE(content.find("desktopicon"), std::string::npos)
      << "Inno Setup must have desktopicon task";
  const bool has_user_desktop =
      content.find("{userdesktop}") != std::string::npos;
  const bool has_auto_desktop =
      content.find("{autodesktop}") != std::string::npos;
  EXPECT_TRUE(has_user_desktop || has_auto_desktop)
      << "Inno Setup must create desktop shortcut";
}

TEST_F(PackagingTest, InnoSetupHasCorrectIcon) {
  // Verify Inno Setup uses the generated icon
  auto inno_script = project_root_ / "installer" / "markdown-viewer.iss";
  std::string content = ReadFile(inno_script);

  EXPECT_FALSE(content.empty()) << "Could not read Inno Setup script";
  const bool uses_relative_project_icon =
      content.find("SetupIconFile=..\\resources\\icon.ico") !=
      std::string::npos;
  const bool uses_working_dir_icon =
      content.find("SetupIconFile=resources\\icon.ico") != std::string::npos;
  EXPECT_TRUE(uses_relative_project_icon || uses_working_dir_icon)
      << "Inno Setup must use resources/icon.ico for setup icon";
}

TEST_F(PackagingTest, IconGeneratorScriptExists) {
  // Verify icon generation script exists
  auto gen_script = project_root_ / "generate-icons.bat";
  EXPECT_TRUE(std::filesystem::exists(gen_script))
      << "Icon generation script must exist at generate-icons.bat";
}

TEST_F(PackagingTest, IconGeneratorUsesMagickConvert) {
  // Verify icon generator uses modern 'magick convert' syntax
  auto gen_script = project_root_ / "generate-icons.bat";
  std::string content = ReadFile(gen_script);

  EXPECT_FALSE(content.empty()) << "Could not read icon generator script";
  EXPECT_NE(content.find("magick"), std::string::npos)
      << "Icon generator must use 'magick' command";
}

TEST_F(PackagingTest, ResourcesInstalledToCorrectLocation) {
  // Verify that README and images are installed to correct locations
  auto cmake_file = project_root_ / "CMakeLists.txt";
  std::string content = ReadFile(cmake_file);

  EXPECT_FALSE(content.empty()) << "Could not read CMakeLists.txt";

  // Images should go to resources/ folder
  EXPECT_NE(content.find("DESTINATION resources"), std::string::npos)
      << "Images must be installed to resources/ folder";
}

TEST_F(PackagingTest, UserReadmeExists) {
  // Verify that user-focused README exists in resources
  auto user_readme = project_root_ / "resources" / "README.md";
  EXPECT_TRUE(std::filesystem::exists(user_readme))
      << "User README must exist at resources/README.md";

  std::string content = ReadFile(user_readme);
  EXPECT_FALSE(content.empty()) << "User README must not be empty";

  // User README should have user-focused content
  EXPECT_NE(content.find("Features"), std::string::npos)
      << "User README should have a Features section";
  EXPECT_NE(content.find("License"), std::string::npos)
      << "User README should have a License section";
}

TEST_F(PackagingTest, DefaultDocumentLoadingOrder) {
  // Verify that the program looks for README in correct locations
  // Order should be: exe dir, parent dir, current dir
  auto wx_app_file = project_root_ / "src" / "app" / "wx_app.cc";
  std::string content = ReadFile(wx_app_file);

  EXPECT_FALSE(content.empty()) << "Could not read wx_app.cc";

  // Should look in executable directory first (portable)
  EXPECT_NE(content.find("executable_path.parent_path()"), std::string::npos)
      << "Should look for README in executable directory";
}

TEST_F(PackagingTest, NoNSISReferences) {
  // Verify no NSIS references remain in Packaging.cmake
  auto packaging_file = project_root_ / "cmake" / "Packaging.cmake";
  std::string content = ReadFile(packaging_file);

  EXPECT_FALSE(content.empty()) << "Could not read Packaging.cmake";

  // Should NOT have NSIS generator
  EXPECT_EQ(content.find("CPACK_GENERATOR \"NSIS"), std::string::npos)
      << "NSIS should not be in CPACK_GENERATOR";
}

TEST_F(PackagingTest, WindowsRcPrefersAppIconOverWxDefaultIcon) {
  const auto rc_file = project_root_ / "src" / "app" / "markdown_viewer.rc";
  const std::string content = ReadFile(rc_file);

  EXPECT_FALSE(content.empty()) << "Could not read markdown_viewer.rc";

  // What we learned:
  // wxWidgets' wx.rc defines wxICON_AAA and Explorer can pick that icon unless
  // the app defines its own icon resource with a name that sorts earlier.
  const size_t app_icon_pos = content.find("aaa ICON");
  const size_t wx_include_pos = content.find("#include <wx/msw/wx.rc>");
  EXPECT_NE(app_icon_pos, std::string::npos)
      << "App icon resource ('aaa ICON ...') must be defined.";
  EXPECT_NE(wx_include_pos, std::string::npos)
      << "wxWidgets resource include must be present.";
  EXPECT_LT(app_icon_pos, wx_include_pos)
      << "App icon resource must appear before including wx/msw/wx.rc.";
  EXPECT_NE(content.find("resources/icon.ico"), std::string::npos)
      << "App icon resource must point to resources/icon.ico.";

  // Manual verification tip:
  // Explorer icon cache can show stale icons for the same path. To verify the
  // fix, copy markdown_viewer.exe to a different folder/name and check
  // Properties there.
}

TEST_F(PackagingTest, WindowsPresetsUseStaticVcpkgTriplet) {
  const auto presets_file = project_root_ / "CMakePresets.json";
  const std::string content = ReadFile(presets_file);

  EXPECT_FALSE(content.empty()) << "Could not read CMakePresets.json";
  EXPECT_NE(content.find("\"windows-msvc-debug\""), std::string::npos);
  EXPECT_NE(content.find("\"windows-msvc-release\""), std::string::npos);
  EXPECT_NE(content.find("\"windows-clangcl-debug\""), std::string::npos);
  EXPECT_NE(content.find("\"VCPKG_TARGET_TRIPLET\": \"x64-windows-static\""),
            std::string::npos)
      << "Windows presets must use static triplet for standalone binaries.";
}

TEST_F(PackagingTest, WindowsUsesWxConfigPackageMode) {
  const auto dependencies_file = project_root_ / "cmake" / "Dependencies.cmake";
  const std::string content = ReadFile(dependencies_file);

  EXPECT_FALSE(content.empty()) << "Could not read cmake/Dependencies.cmake";
  EXPECT_NE(content.find("if(WIN32)"), std::string::npos);
  EXPECT_NE(
      content.find(
          "find_package(wxWidgets CONFIG REQUIRED COMPONENTS base core html)"),
      std::string::npos)
      << "Windows must use wxWidgets CONFIG mode so imported targets include "
         "full static transitive dependencies.";
}
