[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [string] $RaylibIncludeDirectory,

    [Parameter(Mandatory)]
    [string] $RaylibLibrary,

    [Parameter(Mandatory)]
    [string] $FfmpegIncludeDirectory,

    [Parameter(Mandatory)]
    [string] $FfmpegLibraryDirectory,

    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string] $Configuration = 'Release',

    [ValidateSet('win-x64')]
    [string] $RuntimeIdentifier = 'win-x64',

    [string] $Generator
)

$ErrorActionPreference = 'Stop'

$repositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$raylibIncludes = (Resolve-Path -LiteralPath $RaylibIncludeDirectory).Path
$raylibImportLibrary = (Resolve-Path -LiteralPath $RaylibLibrary).Path
$ffmpegIncludes = (Resolve-Path -LiteralPath $FfmpegIncludeDirectory).Path
$ffmpegLibraries = (Resolve-Path -LiteralPath $FfmpegLibraryDirectory).Path

if (-not (Test-Path -LiteralPath (Join-Path $raylibIncludes 'raylib.h') -PathType Leaf)) {
    throw "raylib.h was not found in '$raylibIncludes'."
}

if (-not (Test-Path -LiteralPath (Join-Path $ffmpegIncludes 'libavcodec\avcodec.h') -PathType Leaf)) {
    throw "FFmpeg development headers were not found in '$ffmpegIncludes'."
}

$buildDirectory = Join-Path $repositoryRoot "build\csharp-$RuntimeIdentifier"
$installDirectory = Join-Path $repositoryRoot "csharp\native\$RuntimeIdentifier"

$configureArguments = @(
    '-S', $repositoryRoot,
    '-B', $buildDirectory,
    '-DRMEDIA_BUILD_SHARED=ON',
    "-DRAYLIB_INCLUDE_DIR=$raylibIncludes",
    "-DRAYLIB_LIBRARY_RELEASE=$raylibImportLibrary",
    "-DFFMPEG_INCLUDE_DIR=$ffmpegIncludes",
    "-DFFMPEG_LIBRARY_DIR=$ffmpegLibraries",
    "-DCMAKE_INSTALL_PREFIX=$installDirectory"
)

if ($Generator) {
    $configureArguments += @('-G', $Generator)
}

& cmake @configureArguments
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed with exit code $LASTEXITCODE." }

& cmake --build $buildDirectory --config $Configuration
if ($LASTEXITCODE -ne 0) { throw "Native build failed with exit code $LASTEXITCODE." }

& cmake --install $buildDirectory --config $Configuration
if ($LASTEXITCODE -ne 0) { throw "Native install failed with exit code $LASTEXITCODE." }

$installedLibrary = Join-Path $installDirectory 'bin\raymedia.dll'
if (-not (Test-Path -LiteralPath $installedLibrary -PathType Leaf)) {
    throw "The native build completed, but '$installedLibrary' was not installed."
}

$packageRuntimeDirectory = Join-Path $repositoryRoot "csharp\RaylibMedia\runtimes\$RuntimeIdentifier\native"
New-Item -ItemType Directory -Path $packageRuntimeDirectory -Force | Out-Null
Copy-Item -LiteralPath $installedLibrary -Destination (Join-Path $packageRuntimeDirectory 'raymedia.dll') -Force

Write-Host "raymedia was installed to '$installDirectory'."
Write-Host "The package runtime was refreshed in '$packageRuntimeDirectory'."
Write-Host 'FFmpeg binaries are intentionally not copied into the NuGet package.'
