using System.Runtime.InteropServices;
using Raylib_cs;

namespace RaylibMedia;

internal static class NativeMethods
{
    private const string LibraryName = "raymedia";

    [StructLayout(LayoutKind.Sequential)]
    internal struct NativeMediaStream
    {
        internal Texture2D VideoTexture;
        internal AudioStream AudioStream;
        internal nint Context;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct NativeMediaProperties
    {
        internal double DurationSeconds;
        internal float AverageFramesPerSecond;

        [MarshalAs(UnmanagedType.I1)]
        internal bool HasVideo;

        [MarshalAs(UnmanagedType.I1)]
        internal bool HasAudio;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct NativeMediaStreamReader
    {
        internal nint Read;
        internal nint Seek;
        internal nint UserData;
    }

    internal static NativeMediaStream LoadMedia(string fileName, MediaLoadFlags flags)
    {
        nint utf8FileName = Marshal.StringToCoTaskMemUTF8(fileName);
        try
        {
            return LoadMediaExNative(utf8FileName, (int)flags);
        }
        finally
        {
            Marshal.FreeCoTaskMem(utf8FileName);
        }
    }

    [DllImport(LibraryName, EntryPoint = "LoadMediaEx", CallingConvention = CallingConvention.Cdecl)]
    private static extern NativeMediaStream LoadMediaExNative(nint fileName, int flags);

    [DllImport(LibraryName, EntryPoint = "LoadMediaFromStream", CallingConvention = CallingConvention.Cdecl)]
    internal static extern NativeMediaStream LoadMediaFromStream(NativeMediaStreamReader reader, int flags);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    internal static extern bool IsMediaValid(NativeMediaStream media);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern NativeMediaProperties GetMediaProperties(NativeMediaStream media);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    internal static extern bool UpdateMedia(ref NativeMediaStream media);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    internal static extern bool UpdateMediaEx(ref NativeMediaStream media, double deltaTime);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int GetMediaState(NativeMediaStream media);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int SetMediaState(NativeMediaStream media, int newState);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern double GetMediaPosition(NativeMediaStream media);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    internal static extern bool SetMediaPosition(NativeMediaStream media, double timeSeconds);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    internal static extern bool SetMediaLooping(
        NativeMediaStream media,
        [MarshalAs(UnmanagedType.I1)] bool loop);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int SetMediaFlag(int flag, int value);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern int GetMediaFlag(int flag);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
    internal static extern void UnloadMedia(ref NativeMediaStream media);
}
