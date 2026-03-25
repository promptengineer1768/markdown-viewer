# Development & Build Guide

## Required Tool Versions

These versions are pinned to the lowest common denominator across all supported platforms (Ubuntu 24.04 LTS, Homebrew, Chocolatey).

| Tool          | Version      | Ubuntu 24.04 (apt) | Homebrew   | Chocolatey |
|---------------|-------------|--------------------|------------|------------|
| CMake         | >= 3.28      | 3.28.3             | 4.3.0      | 4.3.0      |
| Ninja         | >= 1.11      | 1.11.1             | 1.13.2     | 1.13.2     |
| Git           | >= 2.43      | 2.43.0             | 2.53.0     | 2.53.0     |
| GCC           | >= 13        | 13.2.0             | 15.2.0     | n/a        |
| Clang         | >= 19        | 19.x (clang-19)    | 22.1.1     | latest     |
| clang-tidy    | >= 19        | 19.x (clang-tools-19) | 22.1.1  | latest     |
| clang-format  | >= 19        | 19.1.1 (clang-format-19) | 22.1.1 | latest |
| wxWidgets     | >= 3.2       | 3.2.x              | 3.2.x      | 3.3.1      |
| GTest         | >= 1.14.0    | 1.14.0             | 1.17.x     | 1.17.x     |

CMake 4.x is supported by this project.
GTest is pinned to >= 1.14.0 to match Ubuntu 24.04's apt package.

On Ubuntu 24.04, clang and g++ are versioned (clang-19, g++-13). CI passes
`-DCMAKE_C_COMPILER=clang-19 -DCMAKE_CXX_COMPILER=clang++-19` for clang builds,
and `-DCMAKE_C_COMPILER=gcc-13 -DCMAKE_CXX_COMPILER=g++-13` for gcc builds.

On macOS, Apple Clang is used by default (no explicit version pin required).

On Windows with MSYS2, GCC is installed via pacman and is typically the latest available.

## Local Development (Windows)

Use the provided batch wrappers:

```
build-msvc-debug.bat      # MSVC 2022 Debug build
build-msvc-release.bat    # MSVC 2022 Release build (for packaging)
build-clang-debug.bat     # Clang-CL Debug build
build-gcc-debug.bat       # MSYS2 UCRT64 GCC Debug build
test-all.bat              # Run unit tests for all active builds
package-windows.bat       # Generate installer (requires Release build)
```

Each batch file:
1. Bootstraps the build environment (installs VS Build Tools / MSYS2 if needed)
2. Sets up vcpkg and dependencies
3. Configures, builds, and tests using CMake presets

Do NOT run cmake directly on Windows. Always use the batch files.

## Local Development (Linux)

```
cmake --preset linux-clang-debug -DCMAKE_C_COMPILER=clang-19 -DCMAKE_CXX_COMPILER=clang++-19
cmake --build --preset build-linux-clang-debug
ctest --preset test-linux-clang-debug
```

## Local Development (macOS)

```
cmake --preset macos-clang-debug
cmake --build --preset build-macos-clang-debug
ctest --preset test-macos-clang-debug
```

## Build System Architecture

The project uses **CMake Presets** (`CMakePresets.json`) to manage build configurations across platforms.

- **vcpkg** is used in manifest mode (`vcpkg.json`) to manage dependencies (wxWidgets, GTest).
- **GitHub Actions** CI mirrors these presets for PR validation and release builds.
- All compiler flags live in the presets, not in the shell scripts.

## Project Structure

- `src/core/`: Library code (parser, exporter, i18n logic). No GUI dependencies.
- `src/app/`: wxWidgets application and main frame.
- `include/markdown/`: Public headers.
- `resources/`: Assets, icons, and locale files.
- `tests/unit/`: GTest unit tests.
- `cmake/`: Helper modules for warnings, packaging, sanitizers, versioning, i18n.
- `scripts/`: Bootstrap, build, and CI scripts.
- `debug/`: Temporary files, logs, local-only tools (gitignored).

## Local CI Simulation

To simulate GitHub Actions locally on Windows, run `tools\local\setup-act.bat`.
This installs `act` and requires Docker Desktop. Only Linux/macOS jobs can be simulated.
This tool is local-only and not part of the official build system.

## Localization

UI strings are stored in `resources/locale/<locale>/ui_strings.txt`. The app automatically detects the system locale using standard C++ and OS-specific APIs.

## Continuous Integration

The CI pipeline (`.github/workflows/ci.yml`) validates builds on:
- **Windows** (MSVC debug, Clang-CL debug, MSYS2 GCC debug)
- **Linux** (Clang debug, GCC debug)
- **macOS** (Clang debug)

Packaging runs only on the main branch or tagged releases.

## Testing Procedures

- Use `test-all.bat` on Windows after a successful build.
- Use `ctest --preset <preset>` on Linux and macOS.
- Keep parser/exporter tests deterministic and free of GUI dependencies.
- GUI-specific behavior (for example high-DPI rendering and theme detection)
  should be validated with targeted manual checks or dedicated GUI tests.

## Known Limitations

- `ParseMarkdown` currently loads the full document into memory and builds the
  complete AST before rendering.
- Very large documents may benefit from a future streaming parser/exporter
  design.

## API Documentation

- Public library API usage is documented in `docs/API.md`.
- Doxygen generation is supported via the repository `Doxyfile`:
  - `doxygen Doxyfile`
