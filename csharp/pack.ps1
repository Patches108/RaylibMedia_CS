[CmdletBinding()]
param(
    [ValidatePattern('^[0-9]+\.[0-9]+\.[0-9]+(?:-[0-9A-Za-z.-]+)?$')]
    [string] $Version = '0.1.0-beta.1',

    [ValidatePattern('^https://github\.com/[^/]+/[^/]+(?:\.git)?$')]
    [string] $RepositoryUrl = 'https://github.com/Patches108/RaylibMedia_CS',

    [string] $OutputDirectory
)

$ErrorActionPreference = 'Stop'

$OutputDirectory = if ($OutputDirectory) { $OutputDirectory } else { Join-Path $PSScriptRoot 'artifacts' }
$repositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$project = Join-Path $PSScriptRoot 'RaylibMedia\RaylibMedia.csproj'
$nativeLibrary = Join-Path $PSScriptRoot 'RaylibMedia\runtimes\win-x64\native\raymedia.dll'
$output = [System.IO.Path]::GetFullPath($OutputDirectory)

if (-not (Test-Path -LiteralPath $nativeLibrary -PathType Leaf)) {
    throw "The package runtime '$nativeLibrary' is missing. Run build-native.ps1 first."
}

New-Item -ItemType Directory -Path $output -Force | Out-Null

$packArguments = @(
    'pack', $project,
    '--configuration', 'Release',
    '--output', $output,
    "-p:PackageVersion=$Version"
)

if ($RepositoryUrl) {
    $packArguments += "-p:RepositoryUrl=$RepositoryUrl"
}

Push-Location $repositoryRoot
try {
    & dotnet @packArguments
    if ($LASTEXITCODE -ne 0) {
        throw "dotnet pack failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}

$package = Join-Path $output "RaylibMedia.CS.$Version.nupkg"
$symbols = Join-Path $output "RaylibMedia.CS.$Version.snupkg"

& (Join-Path $PSScriptRoot 'validate-package.ps1') -PackagePath $package
if ($LASTEXITCODE -ne 0) {
    throw "Package validation failed with exit code $LASTEXITCODE."
}

if (-not (Test-Path -LiteralPath $symbols -PathType Leaf)) {
    throw "The symbol package '$symbols' was not created."
}

Write-Host "NuGet package: $package"
Write-Host "Symbols:      $symbols"
