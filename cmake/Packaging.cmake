# cmake/Packaging.cmake
# CPack configuration for binary packaging.

set(CPACK_PACKAGE_NAME "markdown-viewer")
set(CPACK_PACKAGE_VENDOR "George Halifax")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A modern desktop Markdown viewer/previewer.")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MarkdownViewer")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Output packages to dist/ directory with platform subdirectories
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_SOURCE_DIR}/dist")

if(WIN32)
  # WiX requires a known text extension for the license file.
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
  # Use ZIP for portable package
  # Inno Setup installer is built separately using build-inno-setup.bat
  # This provides better desktop shortcut support than NSIS
  set(CPACK_GENERATOR "ZIP")
  
  # Check if WiX is available for MSI packaging
  find_program(WIX_CANDLE candle.exe PATHS 
    "C:/Program Files (x86)/WiX Toolset v3.14/bin"
    "C:/Program Files/WiX Toolset v3.14/bin"
    ENV PATH
  )
  if(WIX_CANDLE)
    message(STATUS "WiX Toolset found, enabling MSI packaging")
    list(APPEND CPACK_GENERATOR "WIX")
  else()
    message(STATUS "WiX Toolset not found, MSI packaging disabled")
  endif()
  
  set(CPACK_WINDOWS_PACKAGE_NAME "Markdown Viewer")

  # WIX configuration (for MSI packages when WiX is available)
  set(CPACK_WIX_UPGRADE_GUID "2f10b8cf-862d-45db-abe9-1b5ae412d274")
  set(CPACK_WIX_PRODUCT_NAME "Markdown Viewer")
  set(CPACK_WIX_PROGRAM_ICON "${CMAKE_SOURCE_DIR}/resources/icon.ico")
  set(CPACK_WIX_UI "WixUI_InstallDir")
  set(CPACK_WIX_PROPERTY_ALLUSERS "1")
elseif(APPLE)
  set(CPACK_GENERATOR "DragNDrop;TGZ")
  if(TARGET markdown_viewer)
    set_target_properties(markdown_viewer PROPERTIES
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_BUNDLE_NAME "Markdown Viewer"
      MACOSX_BUNDLE_GUI_IDENTIFIER "com.markdown.viewer"
      MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
      MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2026 George Halifax"
    )
  endif()
else()
  set(CPACK_GENERATOR "DEB;RPM;TGZ")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "George Halifax <halifaxgeorge@gmail.com>")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libwxgtk3.2-1")
  set(CPACK_RPM_PACKAGE_LICENSE "MIT")
  set(CPACK_RPM_PACKAGE_REQUIRES "wxGTK3")
  set(CPACK_RPM_PACKAGE_GROUP "Applications/Editors")
endif()

include(CPack)
