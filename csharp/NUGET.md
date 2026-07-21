# NuGet local testing and owner release guide

> [!IMPORTANT]
> Only the package owner or an explicitly authorized maintainer may upload `RaylibMedia` to
> NuGet.org. Testers and contributors must stop after the **Local testing workflow**. They do not
> need a NuGet.org account or API key and must never run `dotnet nuget push` for this package.

Run commands from the repository root in a normal PowerShell terminal.

## Local testing workflow

This workflow creates a package only on the tester's computer. Nothing is uploaded.

### 1. Build and run the ABI tests

```powershell
dotnet build .\RaylibMedia.sln --configuration Release
dotnet run --project .\csharp\RaylibMedia.Tests\RaylibMedia.Tests.csproj --configuration Release
```

The smoke test builds a small native stub and checks the actual C# P/Invoke ABI without opening a
Raylib window.

### 2. Create and validate a local package

```powershell
$Version = '0.1.0-beta.1'
.\csharp\pack.ps1 -Version $Version
```

This produces local, ignored build artifacts:

- `csharp\artifacts\RaylibMedia.<version>.nupkg`
- `csharp\artifacts\RaylibMedia.<version>.snupkg`

The validator checks the managed assembly, XML documentation, Windows x64 runtime, README, icon,
license, notices, and Raylib-cs dependency. It also rejects accidental FFmpeg DLL inclusion.

### 3. Install from the local folder

Create a disposable project outside this repository and use only the local folder as the source:

```powershell
$LocalFeed = (Resolve-Path '.\csharp\artifacts').Path
$TestRoot = Join-Path $env:TEMP 'RaylibMediaPackageTest'

dotnet new console -n RaylibMediaPackageTest -o $TestRoot -f net9.0
dotnet add "$TestRoot\RaylibMediaPackageTest.csproj" package RaylibMedia `
  --version $Version `
  --source $LocalFeed
dotnet build "$TestRoot\RaylibMediaPackageTest.csproj" -r win-x64
```

This does not contact NuGet.org for `RaylibMedia`. Package dependencies may still be restored from
the tester's configured sources.

For a playback test, add a media file and put the five required FFmpeg 7 DLLs beside the test
executable. The package deliberately does not redistribute FFmpeg.

### 4. Report the result

Testers should report:

- Windows version and process architecture.
- .NET SDK version from `dotnet --version`.
- The RaylibMedia version or commit tested.
- Whether build, ABI tests, package install, native loading, playback, seeking, looping, and disposal
  succeeded.
- The complete exception and Raylib log output if something failed.

## Testers and contributors must not

- Upload `.nupkg` or `.snupkg` files to NuGet.org or another public registry as an official release.
- Create or request an API key for the owner's NuGet package.
- Publish a modified build using the `RaylibMedia` package ID.
- Commit generated packages, FFmpeg binaries, `bin`, `obj`, or `artifacts` directories.
- Share locally built packages as though they were releases from DarkSoft/Patches108.

Fork owners may publish independently only under a different package ID and must follow all license,
attribution, security, and dependency obligations.

---

## Package-owner release workflow

Everything below this point is for DarkSoft/Patches108 or another explicitly authorized package
maintainer. Testers should stop here.

### 1. Set the release values

```powershell
$Version = '0.1.0-beta.1'
$RepositoryUrl = 'https://github.com/Patches108/RaylibMedia_CS'
```

Published NuGet versions are immutable. Never reuse a version that has already been uploaded.

### 2. Refresh the native runtime when C code changes

`build-native.ps1` builds and installs the C DLL, then copies it into the package's
`runtimes/win-x64/native` directory:

```powershell
.\csharp\build-native.ps1 `
  -RaylibIncludeDirectory 'D:\path\to\raylib-6.0\src' `
  -RaylibLibrary 'D:\path\to\raylib-6.0\build-shared\raylib\Release\raylib.lib' `
  -FfmpegIncludeDirectory 'D:\path\to\ffmpeg-7.1-shared\include' `
  -FfmpegLibraryDirectory 'D:\path\to\ffmpeg-7.1-shared\lib'
```

Skip this step when only managed code or documentation changed and the checked-in native DLL is
still current.

### 3. Build, test, pack, and inspect

Repeat the complete local testing workflow above, then build the final package from the exact Git
commit or tag intended for release:

```powershell
.\csharp\pack.ps1 -Version $Version -RepositoryUrl $RepositoryUrl
```

Inspect the package metadata and README preview before publishing. Confirm that the repository URL,
commit, version, dependencies, runtime files, license, icon, and notices are correct.

### 4. Upload to NuGet.org — owner only

For the first release, use NuGet.org's **Upload Package** page and inspect its validation preview
before submitting the `.nupkg`.

For later command-line releases, use an owner-created, package-scoped API key:

```powershell
dotnet nuget push ".\csharp\artifacts\RaylibMedia.$Version.nupkg" `
  --api-key $env:NUGET_API_KEY `
  --source https://api.nuget.org/v3/index.json
```

Never put an API key in this repository, a script, terminal screenshot, chat message, build log, or
GitHub issue. Revoke it immediately if it is exposed.

Until the package owner has claimed the package ID with the first NuGet.org release, do not ask
public testers to upload anything or distribute the package as an official release.
