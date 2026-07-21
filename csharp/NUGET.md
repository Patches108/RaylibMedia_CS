# Build and publish the RaylibMedia NuGet package

This is the release checklist for the package owner. Run all commands from the repository root in a
normal PowerShell terminal.

## 1. Set the release values

Use the URL of the GitHub repository that will contain the exact source being packaged:

```powershell
$Version = '0.1.0-beta.1'
$RepositoryUrl = 'https://github.com/Patches108/RaylibMedia_CS'
```

Do not reuse a version after uploading it to NuGet.org. Published package versions are immutable.

## 2. Refresh the native runtime when C code changes

`build-native.ps1` builds and installs the C DLL, then copies it into the package's
`runtimes/win-x64/native` directory automatically:

```powershell
.\csharp\build-native.ps1 `
  -RaylibIncludeDirectory 'D:\path\to\raylib-6.0\src' `
  -RaylibLibrary 'D:\path\to\raylib-6.0\build-shared\raylib\Release\raylib.lib' `
  -FfmpegIncludeDirectory 'D:\path\to\ffmpeg-7.1-full_build-shared\include' `
  -FfmpegLibraryDirectory 'D:\path\to\ffmpeg-7.1-full_build-shared\lib'
```

Skip this step when only managed code or documentation changed and the checked-in native DLL is
still current.

## 3. Build and test the solution

```powershell
dotnet build .\RaylibMedia.sln --configuration Release
dotnet run --project .\csharp\RaylibMedia.Tests\RaylibMedia.Tests.csproj --configuration Release
```

The smoke test builds a tiny native stub and checks the actual C# P/Invoke ABI without opening a
Raylib window.

## 4. Create and validate both packages

```powershell
.\csharp\pack.ps1 -Version $Version -RepositoryUrl $RepositoryUrl
```

This produces and validates:

- `csharp\artifacts\RaylibMedia.<version>.nupkg`
- `csharp\artifacts\RaylibMedia.<version>.snupkg`

The validator checks the managed assembly, XML documentation, Windows x64 runtime, readme, license,
notices, and Raylib-cs dependency. It also rejects accidental FFmpeg DLL inclusion.

## 5. Test from the local package folder

Create a disposable project outside this repository, then install only from the local feed:

```powershell
dotnet new console -n RaylibMediaPackageTest -f net9.0
Set-Location .\RaylibMediaPackageTest
dotnet add package RaylibMedia --version $Version --source 'D:\path\to\raylib-media\csharp\artifacts'
dotnet build -r win-x64
```

For a playback test, add a video and put the five required FFmpeg 7 DLLs beside the test executable.
The package deliberately does not redistribute FFmpeg.

## 6. Upload to NuGet.org

The safest first upload is through NuGet.org's **Upload Package** page: select the `.nupkg`, inspect
the metadata/readme preview, and submit it. Upload the `.snupkg` through the same publishing flow if
it is not accepted automatically by your chosen client.

For command-line publishing, create a scoped NuGet.org API key and keep it out of source control:

```powershell
dotnet nuget push ".\csharp\artifacts\RaylibMedia.$Version.nupkg" `
  --api-key $env:NUGET_API_KEY `
  --source https://api.nuget.org/v3/index.json
```

Never put the API key in this repository, a script, a command screenshot, or a GitHub issue.
