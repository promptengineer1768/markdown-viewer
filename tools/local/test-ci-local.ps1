# tools/local/test-ci-local.ps1
# Local CI simulation for Windows
# Mirrors GitHub Actions workflow steps locally.

param(
    [string]$Compiler = "msvc",
    [string]$Config = "debug",
    [switch]$SkipFormat,
    [switch]$SkipBuild,
    [switch]$SkipTest,
    [switch]$SkipTidy,
    [switch]$Help
)

$ErrorActionPreference = "Stop"
$ProjectRoot = $PSScriptRoot | Split-Path | Split-Path
Set-Location $ProjectRoot

$PassCount = 0
$FailCount = 0

function Write-Step {
    param([string]$Name)
    Write-Host "`n=== $Name ===" -ForegroundColor Cyan
}

function Write-Pass {
    param([string]$Message)
    Write-Host "[PASS] $Message" -ForegroundColor Green
    $script:PassCount++
}

function Write-Fail {
    param([string]$Message)
    Write-Host "[FAIL] $Message" -ForegroundColor Red
    $script:FailCount++
}

function Write-Skip {
    param([string]$Message)
    Write-Host "[SKIP] $Message" -ForegroundColor Yellow
}

if ($Help) {
    Write-Host @"

Usage: test-ci-local.ps1 [options]

Options:
    -Compiler <compiler>  Compiler to use: msvc (default), clang-cl, gcc
    -Config <config>      Build configuration: debug (default), release
    -SkipFormat           Skip format check
    -SkipBuild            Skip configure and build
    -SkipTest             Skip tests
    -SkipTidy             Skip clang-tidy check
    -Help                 Show this help

Examples:
    test-ci-local.ps1
    test-ci-local.ps1 -Compiler clang-cl -Config release
    test-ci-local.ps1 -SkipTidy

"@
    exit 0
}

Write-Host "=== Local CI Simulation ===" -ForegroundColor Cyan
Write-Host "Compiler: $Compiler"
Write-Host "Config:   $Config"
Write-Host ""

# ============================================================
# Step 1: Format Check
# ============================================================
if ($SkipFormat) {
    Write-Skip "Format check"
} else {
    Write-Step "Format Check"

    # Find clang-format
    $ClangFormatPaths = @(
        "C:\Program Files\LLVM\bin\clang-format.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\bin\clang-format.exe"
    )
    
    $ClangFormat = $null
    foreach ($path in $ClangFormatPaths) {
        if (Test-Path $path) {
            $ClangFormat = $path
            break
        }
    }
    
    if (-not $ClangFormat) {
        $ClangFormat = Get-Command clang-format -ErrorAction SilentlyContinue
        if ($ClangFormat) {
            $ClangFormat = $ClangFormat.Source
        }
    }

    if (-not $ClangFormat) {
        Write-Fail "clang-format not found"
        return
    }

    Write-Host "Using: $ClangFormat"
    & $ClangFormat --version
    Write-Host ""

    $FormatErrors = 0
    $Files = Get-ChildItem -Path "src", "include", "tests" -Include "*.cc", "*.h" -Recurse
    
    foreach ($file in $Files) {
        $result = & $ClangFormat --dry-run --Werror --style=Google $file.FullName 2>&1
        if ($LASTEXITCODE -ne 0) {
            Write-Host "  ERROR: $($file.FullName) needs formatting"
            $FormatErrors++
        }
    }

    if ($FormatErrors -eq 0) {
        Write-Pass "Format check - all files formatted correctly"
    } else {
        Write-Fail "Format check - $FormatErrors files need formatting"
        Write-Host "  Run: $ClangFormat -i --style=Google <file>"
    }
}

# ============================================================
# Step 2: Configure
# ============================================================
if ($SkipBuild) {
    Write-Skip "Configure and build"
} else {
    Write-Step "Configure"

    $PresetMap = @{
        "msvc-debug" = "windows-msvc-debug"
        "msvc-release" = "windows-msvc-release"
        "clang-cl-debug" = "windows-clangcl-debug"
        "clang-cl-release" = "windows-clangcl-release"
        "gcc-debug" = "windows-gcc-debug"
        "gcc-release" = "windows-gcc-release"
    }

    $PresetKey = "$Compiler-$Config"
    if (-not $PresetMap.ContainsKey($PresetKey)) {
        Write-Fail "Unknown compiler/config: $PresetKey"
        Write-Host "  Supported: msvc, clang-cl, gcc (debug/release)"
        exit 1
    }

    $Preset = $PresetMap[$PresetKey]
    Write-Host "Using preset: $Preset"

    # Clean build directory
    $BuildDir = Join-Path "build" $Preset
    if (Test-Path $BuildDir) {
        Write-Host "Cleaning build directory..."
        Remove-Item -Path $BuildDir -Recurse -Force
    }

    # Configure
    & cmake --preset $Preset
    if ($LASTEXITCODE -ne 0) {
        Write-Fail "Configure failed"
        exit 1
    }
    Write-Pass "Configure succeeded"

    # ============================================================
    # Step 3: Build
    # ============================================================
    Write-Step "Build"

    $BuildPresetMap = @{
        "msvc-debug" = "build-windows-msvc-debug"
        "msvc-release" = "build-windows-msvc-release"
        "clang-cl-debug" = "build-windows-clangcl-debug"
        "clang-cl-release" = "build-windows-clangcl-release"
        "gcc-debug" = "build-windows-gcc-debug"
        "gcc-release" = "build-windows-gcc-release"
    }

    $BuildPreset = $BuildPresetMap[$PresetKey]
    & cmake --build --preset $BuildPreset
    if ($LASTEXITCODE -ne 0) {
        Write-Fail "Build failed"
        exit 1
    }
    Write-Pass "Build succeeded"
}

# ============================================================
# Step 4: Test
# ============================================================
if ($SkipTest -or $SkipBuild) {
    if ($SkipTest) {
        Write-Skip "Test"
    } else {
        Write-Skip "Test (build was skipped)"
    }
} else {
    Write-Step "Test"

    $TestPresetMap = @{
        "msvc-debug" = "test-windows-msvc-debug"
        "msvc-release" = "test-windows-msvc-release"
        "clang-cl-debug" = "test-windows-clangcl-debug"
        "clang-cl-release" = "test-windows-clangcl-release"
        "gcc-debug" = "test-windows-gcc-debug"
        "gcc-release" = "test-windows-gcc-release"
    }

    $TestPreset = $TestPresetMap[$PresetKey]
    & ctest --preset $TestPreset --output-on-failure
    if ($LASTEXITCODE -ne 0) {
        Write-Fail "Tests failed"
        exit 1
    }
    Write-Pass "All tests passed"
}

# ============================================================
# Step 5: clang-tidy (optional)
# ============================================================
if ($SkipTidy) {
    Write-Skip "clang-tidy"
} elseif ($SkipBuild) {
    Write-Skip "clang-tidy (build was skipped)"
} else {
    Write-Step "clang-tidy Check"

    # Find clang-tidy
    $ClangTidyPaths = @(
        "C:\Program Files\LLVM\bin\clang-tidy.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\bin\clang-tidy.exe"
    )
    
    $ClangTidy = $null
    foreach ($path in $ClangTidyPaths) {
        if (Test-Path $path) {
            $ClangTidy = $path
            break
        }
    }
    
    if (-not $ClangTidy) {
        $ClangTidy = Get-Command clang-tidy -ErrorAction SilentlyContinue
        if ($ClangTidy) {
            $ClangTidy = $ClangTidy.Source
        }
    }

    if (-not $ClangTidy) {
        Write-Skip "clang-tidy not found"
    } else {
        $CompileDb = Join-Path $BuildDir "compile_commands.json"
        if (-not (Test-Path $CompileDb)) {
            Write-Skip "compile_commands.json not found (build required)"
        } else {
            Write-Host "Using: $ClangTidy"
            & $ClangTidy --version
            Write-Host ""

            $TidyErrors = 0
            $SourceFiles = Get-ChildItem -Path "src", "tests" -Include "*.cc" -Recurse
            
            foreach ($file in $SourceFiles) {
                $result = & $ClangTidy -p $BuildDir $file.FullName --quiet 2>&1
                if ($LASTEXITCODE -ne 0) {
                    Write-Host "  WARNING: $($file.FullName) has clang-tidy issues"
                    $TidyErrors++
                }
            }

            if ($TidyErrors -eq 0) {
                Write-Pass "clang-tidy check passed"
            } else {
                Write-Host "[WARN] clang-tidy found $TidyErrors files with issues" -ForegroundColor Yellow
            }
        }
    }
}

# ============================================================
# Summary
# ============================================================
Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Cyan
Write-Host "  Passed: $PassCount"
Write-Host "  Failed: $FailCount"
Write-Host ""

if ($FailCount -eq 0) {
    Write-Host "[SUCCESS] All checks passed!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "[FAILURE] Some checks failed" -ForegroundColor Red
    exit 1
}
