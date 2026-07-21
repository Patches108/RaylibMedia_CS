[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [string] $PackagePath
)

$ErrorActionPreference = 'Stop'

$resolvedPackage = (Resolve-Path -LiteralPath $PackagePath).Path
Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive = [System.IO.Compression.ZipFile]::OpenRead($resolvedPackage)

try {
    $entries = @($archive.Entries | ForEach-Object { $_.FullName })
    $requiredEntries = @(
        'lib/net8.0/RaylibMedia.dll',
        'lib/net8.0/RaylibMedia.xml',
        'runtimes/win-x64/native/raymedia.dll',
        'README.md',
        'package-icon.png',
        'THIRD-PARTY-NOTICES.md',
        'LICENSE.md'
    )

    $missingEntries = @($requiredEntries | Where-Object { $_ -notin $entries })
    if ($missingEntries.Count -gt 0) {
        throw "The package is missing required entries: $($missingEntries -join ', ')"
    }

    $forbiddenFfmpegEntries = @($entries | Where-Object {
        $_ -match '(^|/)(avcodec|avformat|avutil|swresample|swscale)-[0-9]+\.dll$'
    })
    if ($forbiddenFfmpegEntries.Count -gt 0) {
        throw "FFmpeg binaries must not be redistributed by this package: $($forbiddenFfmpegEntries -join ', ')"
    }

    $nuspecEntry = $archive.Entries | Where-Object { $_.FullName -like '*.nuspec' } | Select-Object -First 1
    if (-not $nuspecEntry) {
        throw 'The package has no NuSpec manifest.'
    }

    $reader = [System.IO.StreamReader]::new($nuspecEntry.Open())
    try {
        [xml] $nuspec = $reader.ReadToEnd()
    }
    finally {
        $reader.Dispose()
    }

    $metadata = $nuspec.package.metadata
    if ($metadata.id -ne 'RaylibMedia.CS') {
        throw "Unexpected package ID '$($metadata.id)'."
    }

    if ($metadata.license.type -ne 'expression' -or $metadata.license.'#text' -ne 'Zlib') {
        throw 'The package must declare the Zlib license expression.'
    }

    $dependencyIds = @($metadata.dependencies.group.dependency | ForEach-Object { $_.id })
    if ('Raylib-cs' -notin $dependencyIds) {
        throw 'The package does not declare its Raylib-cs dependency.'
    }
}
finally {
    $archive.Dispose()
}

$hash = (Get-FileHash -LiteralPath $resolvedPackage -Algorithm SHA256).Hash
Write-Host "Package validation passed: $resolvedPackage"
Write-Host "SHA-256: $hash"
