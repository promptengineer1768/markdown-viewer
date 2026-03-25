# tools/local/setup-act.ps1
# Installs 'act' for local GitHub Actions simulation.
# This is a local development tool only - NOT part of the official build system.
# All downloaded files go into tools/local/.

$ErrorActionPreference = "Stop"
$toolsDir = $PSScriptRoot

Write-Host "=== Setting up act (GitHub Actions local simulator) ===" -ForegroundColor Cyan

# ============================================================
# Step 1: Update WSL
# ============================================================
Write-Host "`n[1/4] Updating WSL..." -ForegroundColor Cyan

try {
    $wslVersion = wsl --version 2>&1
    Write-Host "WSL is installed:" -ForegroundColor Green
    Write-Host $wslVersion
} catch {
    Write-Host "Installing WSL..." -ForegroundColor Yellow
    wsl --install
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "WSL installation may require a reboot. Please restart and run this script again."
        exit 1
    }
}

Write-Host "Updating WSL..." -ForegroundColor Yellow
wsl --update
if ($LASTEXITCODE -eq 0) {
    Write-Host "WSL updated successfully." -ForegroundColor Green
} else {
    Write-Warning "WSL update failed, but continuing..."
}

# ============================================================
# Step 2: Install Docker Desktop
# ============================================================
Write-Host "`n[2/4] Checking Docker Desktop..." -ForegroundColor Cyan

$dockerDesktopPath = "${env:ProgramFiles}\Docker\Docker\Docker Desktop.exe"
$dockerInstalled = Test-Path $dockerDesktopPath

if (-not $dockerInstalled) {
    Write-Host "Docker Desktop not found. Installing..." -ForegroundColor Yellow
    
    # Download Docker Desktop installer
    $dockerInstallerUrl = "https://desktop.docker.com/win/main/amd64/Docker%20Desktop%20Installer.exe"
    $dockerInstallerPath = Join-Path $toolsDir "DockerDesktopInstaller.exe"
    
    Write-Host "Downloading Docker Desktop installer..." -ForegroundColor Yellow
    Invoke-WebRequest -Uri $dockerInstallerUrl -OutFile $dockerInstallerPath
    
    Write-Host "Installing Docker Desktop (this may take a few minutes)..." -ForegroundColor Yellow
    Write-Host "NOTE: You may need to interact with the installer." -ForegroundColor Yellow
    
    Start-Process -FilePath $dockerInstallerPath -ArgumentList "install", "--quiet", "--accept-license" -Wait
    
    Remove-Item $dockerInstallerPath -Force -ErrorAction SilentlyContinue
    
    Write-Host "Docker Desktop installed." -ForegroundColor Green
    Write-Warning "Docker Desktop may require a reboot. Please restart if Docker doesn't start."
} else {
    Write-Host "Docker Desktop already installed at: $dockerDesktopPath" -ForegroundColor Green
}

# ============================================================
# Step 3: Start Docker
# ============================================================
Write-Host "`n[3/4] Starting Docker..." -ForegroundColor Cyan

# Try to start Docker Desktop if not running
$dockerRunning = Get-Process -Name "Docker Desktop" -ErrorAction SilentlyContinue
if (-not $dockerRunning) {
    Write-Host "Starting Docker Desktop..." -ForegroundColor Yellow
    Start-Process $dockerDesktopPath
    
    # Wait for Docker to be ready (up to 60 seconds)
    $timeout = 60
    $elapsed = 0
    Write-Host "Waiting for Docker to start" -NoNewline
    while ($elapsed -lt $timeout) {
        docker info 2>$null | Out-Null
        if ($LASTEXITCODE -eq 0) {
            Write-Host ""
            Write-Host "Docker is ready!" -ForegroundColor Green
            break
        }
        Write-Host "." -NoNewline
        Start-Sleep -Seconds 2
        $elapsed += 2
    }
    if ($elapsed -ge $timeout) {
        Write-Host ""
        Write-Warning "Docker did not start within $timeout seconds. Please start Docker Desktop manually."
        exit 1
    }
} else {
    Write-Host "Docker Desktop is running." -ForegroundColor Green
}

# Verify Docker is working
docker info 2>$null | Out-Null
if ($LASTEXITCODE -ne 0) {
    Write-Warning "Docker is not responding. Please ensure Docker Desktop is running."
    exit 1
}

# ============================================================
# Step 4: Install act
# ============================================================
Write-Host "`n[4/4] Installing act..." -ForegroundColor Cyan

# Check if act is already installed
$actPath = Get-Command act -ErrorAction SilentlyContinue
if ($actPath) {
    Write-Host "act already installed: $($actPath.Source)" -ForegroundColor Green
    $version = & act --version 2>&1
    Write-Host $version
} else {
    # Download act to tools/local/ (no system-wide install)
    $actExe = Join-Path $toolsDir "act.exe"

    if (Test-Path $actExe) {
        Write-Host "act already present at $actExe" -ForegroundColor Green
    } else {
        Write-Host "Downloading act..." -ForegroundColor Yellow
        $url = "https://github.com/nektos/act/releases/latest/download/act_Windows_x86_64.zip"
        $zipPath = Join-Path $toolsDir "act.zip"

        Invoke-WebRequest -Uri $url -OutFile $zipPath

        # Extract just act.exe
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        $zip = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
        foreach ($entry in $zip.Entries) {
            if ($entry.Name -eq "act.exe") {
                [System.IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $actExe, $true)
                break
            }
        }
        $zip.Dispose()
        Remove-Item $zipPath -Force

        Write-Host "act installed to $actExe" -ForegroundColor Green
    }

    # Add tools/local to PATH for this session only
    $env:Path = "$toolsDir;$env:Path"
}

# ============================================================
# Usage Instructions
# ============================================================
Write-Host "`n=== Usage ===" -ForegroundColor Cyan
Write-Host @"

Run from project root:

  # Simulate Linux clang build:
  tools\local\act.exe -W .github/workflows/ci.yml -j linux-clang --container-architecture linux/amd64 --bind

  # List available jobs:
  tools\local\act.exe -W .github/workflows/ci.yml -l

  # Run all Linux jobs:
  tools\local\act.exe -W .github/workflows/ci.yml --container-architecture linux/amd64 --bind

  # Run local CI simulation (Windows):
  tools\local\test-ci-local.bat

NOTE: Windows/macOS jobs cannot be simulated with act.
Use test-ci-local.bat for Windows builds, or act for Linux/macOS simulation.

"@ -ForegroundColor Gray

Write-Host "=== setup-act complete ===" -ForegroundColor Green
