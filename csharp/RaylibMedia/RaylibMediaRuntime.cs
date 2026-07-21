using System.Collections.ObjectModel;
using System.Runtime.InteropServices;

namespace RaylibMedia;

/// <summary>Describes the native runtime required by this RaylibMedia package.</summary>
public static class RaylibMediaRuntime
{
    private static readonly ReadOnlyCollection<string> NativeLibraries = Array.AsReadOnly(
    [
        "raymedia.dll",
        "raylib.dll",
        "avcodec-61.dll",
        "avformat-61.dll",
        "avutil-59.dll",
        "swscale-8.dll",
        "swresample-5.dll"
    ]);

    /// <summary>Gets whether this package contains a native runtime for the current process.</summary>
    public static bool IsSupportedPlatform =>
        OperatingSystem.IsWindows() && RuntimeInformation.ProcessArchitecture == Architecture.X64;

    /// <summary>Gets the native library filenames required by the Windows x64 runtime.</summary>
    public static IReadOnlyList<string> RequiredNativeLibraries => NativeLibraries;

    /// <summary>Throws when this package does not contain a runtime for the current platform.</summary>
    /// <exception cref="PlatformNotSupportedException">
    /// The process is not a 64-bit Windows process.
    /// </exception>
    public static void EnsureSupportedPlatform()
    {
        if (!IsSupportedPlatform)
        {
            throw new PlatformNotSupportedException(
                $"RaylibMedia 0.1 currently supports Windows x64 only. " +
                $"The current process is {RuntimeInformation.OSDescription} {RuntimeInformation.ProcessArchitecture}.");
        }
    }
}
