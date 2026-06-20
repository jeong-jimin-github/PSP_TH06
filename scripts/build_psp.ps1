[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$SourceDir,
    [string]$WslDistro = "Ubuntu",
    [string]$Pspdev = "/usr/local/pspdev",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot "build\psp"
$packageDir = Join-Path $repoRoot "build\psp-package\PSP\GAME\TH06"
$hostExtractor = Join-Path $repoRoot "build\host-tools\pbg3_extract"

function Convert-ToWslPath([string]$Path) {
    $fullPath = [IO.Path]::GetFullPath($Path)
    $driveRoot = [IO.Path]::GetPathRoot($fullPath)
    if ([string]::IsNullOrWhiteSpace($driveRoot) -or $driveRoot.Length -lt 2) {
        throw "Could not translate the path for WSL: $fullPath"
    }
    $driveLetter = $driveRoot.Substring(0, 1).ToLowerInvariant()
    $relativePath = $fullPath.Substring($driveRoot.Length).Replace('\', '/')
    return "/mnt/$driveLetter/$relativePath"
}

if (-not (Test-Path -LiteralPath $SourceDir -PathType Container)) {
    throw "TH06 source directory does not exist: $SourceDir"
}

$requiredData = @("CM.DAT", "ED.DAT", "IN.DAT", "MD.DAT", "ST.DAT", "TL.DAT")
foreach ($name in $requiredData) {
    $path = Join-Path $SourceDir $name
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Missing original game data: $path"
    }
}

$bgmDir = Join-Path $SourceDir "bgm"
$bgmFiles = @(Get-ChildItem -LiteralPath $bgmDir -Filter "th06_*.wav" -File)
if ($bgmFiles.Count -ne 17) {
    throw "Expected 17 original BGM WAV files in $bgmDir, found $($bgmFiles.Count)."
}

if (-not $SkipBuild) {
    $wslRepo = Convert-ToWslPath $repoRoot

    $buildCommand = @"
set -e
export PSPDEV='$Pspdev'
export PATH='$Pspdev/bin:/usr/local/bin:/usr/bin:/bin'
cd '$wslRepo'
psp-cmake -S . -B build/psp -DCMAKE_BUILD_TYPE=Release
cmake --build build/psp -j2
mkdir -p build/host-tools
g++ -std=c++20 -O2 -Isrc tools/pbg3_extract.cpp src/pbg3/FileAbstraction.cpp src/pbg3/IPbg3Parser.cpp src/pbg3/Pbg3Archive.cpp src/pbg3/Pbg3Parser.cpp -o build/host-tools/pbg3_extract
"@

    & wsl.exe -d $WslDistro -- bash -lc $buildCommand
    if ($LASTEXITCODE -ne 0) {
        throw "PSP cross-build failed with exit code $LASTEXITCODE."
    }
}

$eboot = Join-Path $buildDir "EBOOT.PBP"
if (-not (Test-Path -LiteralPath $eboot -PathType Leaf)) {
    throw "EBOOT.PBP was not found. Build the project first: $eboot"
}

$resolvedRepoRoot = [IO.Path]::GetFullPath($repoRoot).TrimEnd([IO.Path]::DirectorySeparatorChar) +
    [IO.Path]::DirectorySeparatorChar
$resolvedPackageDir = [IO.Path]::GetFullPath($packageDir)
if (-not $resolvedPackageDir.StartsWith($resolvedRepoRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing to clean a package directory outside the workspace: $resolvedPackageDir"
}
if (Test-Path -LiteralPath $resolvedPackageDir -PathType Container) {
    Remove-Item -LiteralPath $resolvedPackageDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $packageDir | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $packageDir "bgm") | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $packageDir "replay") | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $packageDir "data") | Out-Null

if (-not (Test-Path -LiteralPath $hostExtractor -PathType Leaf)) {
    throw "PBG3 extractor was not found. Build without -SkipBuild first: $hostExtractor"
}

$wslExtractor = Convert-ToWslPath $hostExtractor
foreach ($name in $requiredData) {
    $archivePath = Join-Path $SourceDir $name
    $archiveStem = [IO.Path]::GetFileNameWithoutExtension($name)
    $assetDir = Join-Path $packageDir "data\$archiveStem"
    New-Item -ItemType Directory -Force -Path $assetDir | Out-Null

    & wsl.exe -d $WslDistro -- $wslExtractor (Convert-ToWslPath $archivePath) (Convert-ToWslPath $assetDir)
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to extract $name for the PSP sidecar asset fallback."
    }
}

Copy-Item -LiteralPath $eboot -Destination (Join-Path $packageDir "EBOOT.PBP") -Force
foreach ($name in $requiredData) {
    Copy-Item -LiteralPath (Join-Path $SourceDir $name) -Destination (Join-Path $packageDir $name) -Force
}
foreach ($bgm in $bgmFiles) {
    Copy-Item -LiteralPath $bgm.FullName -Destination (Join-Path $packageDir "bgm\$($bgm.Name)") -Force
}

$fontCandidates = @(
    (Join-Path $SourceDir "msgothic.ttc"),
    "C:\Windows\Fonts\msgothic.ttc",
    (Join-Path $SourceDir "NotoSansJP-Regular.ttf")
)
$font = $fontCandidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
if ($null -eq $font) {
    throw "No Japanese font found. Put msgothic.ttc or NotoSansJP-Regular.ttf in $SourceDir."
}

$fontName = if ([IO.Path]::GetFileName($font) -eq "NotoSansJP-Regular.ttf") {
    "NotoSansJP-Regular.ttf"
} else {
    "msgothic.ttc"
}
Copy-Item -LiteralPath $font -Destination (Join-Path $packageDir $fontName) -Force

$exePath = Join-Path $SourceDir "東方紅魔郷.exe"
if (Test-Path -LiteralPath $exePath -PathType Leaf) {
    $hash = (Get-FileHash -LiteralPath $exePath -Algorithm SHA256).Hash.ToLowerInvariant()
    $expected = "9f76483c46256804792399296619c1274363c31cd8f1775fafb55106fb852245"
    if ($hash -ne $expected) {
        Write-Warning "The executable is not the upstream-expected TH06 1.02h hash. It is not copied or executed; only the supplied data files are packaged. SHA-256: $hash"
    }
}

$packageSize = (Get-ChildItem -LiteralPath $packageDir -File -Recurse | Measure-Object Length -Sum).Sum
Write-Host "PSP package ready: $packageDir"
Write-Host ("Package size: {0:N1} MiB" -f ($packageSize / 1MB))
