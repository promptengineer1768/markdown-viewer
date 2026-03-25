cmake_minimum_required(VERSION 3.28)
project(markdown_viewer VERSION 1.0.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(ProjectOptions)
include(Dependencies)
include(CompilerWarnings)
include(Versioning)
include(Sanitizers)
include(CodeCoverage)
include(LinkTimeOptimization)

set(MARKDOWN_CORE_SOURCES
    src/core/file_io.cc
    src/core/html_exporter.cc
    src/core/markdown_parser.cc
    src/core/path_utils.cc
    src/core/text_utils.cc
    src/core/ui_strings.cc
    src/core/locale_file_parser.cc
)

add_library(markdown_core STATIC ${MARKDOWN_CORE_SOURCES})
target_include_directories(markdown_core PUBLIC include)
target_compile_features(markdown_core PUBLIC cxx_std_20)
set_project_warnings(markdown_core)
EnableSanitizers(markdown_core)
EnableCoverage(markdown_core)
EnableLTO(markdown_core)
GenerateVersionHeader(markdown_core)

if(WIN32)
    target_compile_definitions(markdown_core PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
if(MSVC)
    target_compile_options(markdown_core PRIVATE /utf-8)
endif()

if(WIN32)
    add_executable(markdown_viewer WIN32
        src/app/wx_app.cc
        src/app/markdown_viewer.rc
    )
else()
    add_executable(markdown_viewer
        src/app/wx_app.cc
    )
    if(APPLE)
        set_target_properties(markdown_viewer PROPERTIES MACOSX_BUNDLE TRUE)
    endif()
endif()

if(TARGET wx::core AND TARGET wx::base AND TARGET wx::html)
    target_link_libraries(markdown_viewer PRIVATE markdown_core wx::core wx::base wx::html)
else()
    target_link_libraries(markdown_viewer PRIVATE markdown_core ${wxWidgets_LIBRARIES})
endif()
set_project_warnings(markdown_viewer)
EnableSanitizers(markdown_viewer)
EnableCoverage(markdown_viewer)
EnableLTO(markdown_viewer)
GenerateVersionHeader(markdown_viewer)

if(MSVC)
    target_compile_options(markdown_viewer PRIVATE /utf-8)
endif()

if(MARKDOWN_VIEWER_BUILD_TESTS)
    enable_testing()
    include(GoogleTest)

add_executable(markdown_viewer_tests
tests/unit/html_exporter_test.cc
tests/unit/markdown_parser_test.cc
tests/unit/path_utils_test.cc
tests/unit/ui_strings_test.cc
tests/unit/packaging_test.cc
tests/unit/file_io_test.cc
tests/unit/text_utils_test.cc
tests/unit/integration_test.cc
tests/unit/locale_file_parser_test.cc
)
target_link_libraries(markdown_viewer_tests
PRIVATE GTest::gtest_main GTest::gtest markdown_core
)
set_project_warnings(markdown_viewer_tests)
EnableSanitizers(markdown_viewer_tests)
EnableCoverage(markdown_viewer_tests)
if(MSVC)
        target_compile_options(markdown_viewer_tests PRIVATE /utf-8)
    endif()

    gtest_discover_tests(markdown_viewer_tests)
AddCoverageTarget()
endif()

if(TARGET markdown_viewer)
  if(WIN32)
    install(TARGETS markdown_viewer
      RUNTIME DESTINATION bin
    )
  else()
    install(TARGETS markdown_viewer
      RUNTIME DESTINATION bin
      BUNDLE DESTINATION .
    )
  endif()

  # Install screenshot images referenced in README (to resources/ folder to match README paths)
  install(FILES
    ${CMAKE_SOURCE_DIR}/resources/MD_Viewer_Dark_Theme.jpg
    ${CMAKE_SOURCE_DIR}/resources/MD_Viewer_Light_Theme.jpg
    DESTINATION resources
  )

  # Install user-focused README.md as the default document
  install(FILES
    ${CMAKE_SOURCE_DIR}/resources/README.md
    DESTINATION .
    RENAME README.md
  )

  # On Windows, install additional resources.
  # Runtime DLLs are collected via RUNTIME_DEPENDENCIES above.
  if(WIN32)
    # Install locales on Windows
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/locale/ DESTINATION bin/locales)
    
    # Install icon for desktop shortcut
    install(FILES 
      ${CMAKE_SOURCE_DIR}/resources/icon.ico
      DESTINATION bin
    )
  elseif(APPLE)
    # macOS: Install locales in the bundle's Resources
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/locale/ DESTINATION bin/locales)
  else()
    # Linux: Install locales
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/locale/ DESTINATION bin/locales)
  endif()

  add_custom_command(TARGET markdown_viewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:markdown_viewer>/locales
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/locale/ $<TARGET_FILE_DIR:markdown_viewer>/locales
    COMMENT "Copying locales to output directory"
  )
endif()

# Install LICENSE (user README is installed from resources/README.md above)
install(FILES LICENSE DESTINATION .)

if(NOT WIN32 AND NOT APPLE)
    install(FILES resources/markdown-viewer.desktop
        DESTINATION share/applications)
    install(FILES resources/icon.png
        DESTINATION share/icons/hicolor/512x512/apps
        RENAME markdown-viewer.png)
endif()

include(Packaging)

# Add custom target to organize packages into dist/platform/ structure
add_custom_target(package-organized
  COMMAND ${CMAKE_COMMAND} -E echo "Creating organized package structure..."
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/dist/windows
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/dist/linux
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/dist/macos
  COMMAND ${CMAKE_COMMAND} -E echo "Run 'cpack' to generate packages, then 'cmake --build . --target package-organized' to organize them"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Organize packages into dist/ directory"
)
