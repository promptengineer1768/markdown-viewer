# cmake/Dependencies.cmake
# Find and configure external dependencies.

# Prefer CONFIG mode on Windows so imported wx:: targets carry complete
# transitive link dependencies (e.g. static image codec libs).
if(WIN32)
    find_package(wxWidgets CONFIG REQUIRED COMPONENTS base core html)
else()
    find_package(wxWidgets QUIET COMPONENTS base core html)

    if(wxWidgets_FOUND)
        if(DEFINED wxWidgets_USE_FILE AND EXISTS "${wxWidgets_USE_FILE}")
            include(${wxWidgets_USE_FILE})
        endif()
    else()
        find_package(wxWidgets REQUIRED COMPONENTS std html)
    endif()
endif()

if(MARKDOWN_VIEWER_BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)
endif()

if(MARKDOWN_VIEWER_ENABLE_I18N)
    find_package(Gettext QUIET)
endif()
