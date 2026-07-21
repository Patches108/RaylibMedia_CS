using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace RaylibMedia;

internal sealed unsafe class ManagedStreamSource : IDisposable
{
    private const int EndOfFile = -541478725;
    private const int InvalidOperation = -22;
    private const int AvSeekSize = 0x10000;
    private const int AvSeekForce = 0x20000;

    private readonly Stream _stream;
    private readonly bool _leaveOpen;
    private GCHandle _handle;

    internal ManagedStreamSource(Stream stream, bool leaveOpen)
    {
        _stream = stream;
        _leaveOpen = leaveOpen;
        _handle = GCHandle.Alloc(this, GCHandleType.Normal);
    }

    internal NativeMethods.NativeMediaStreamReader CreateReader()
    {
        return new NativeMethods.NativeMediaStreamReader
        {
            Read = (nint)(delegate* unmanaged[Cdecl]<nint, byte*, int, int>)&ReadCallback,
            Seek = _stream.CanSeek
                ? (nint)(delegate* unmanaged[Cdecl]<nint, long, int, long>)&SeekCallback
                : 0,
            UserData = GCHandle.ToIntPtr(_handle),
        };
    }

    public void Dispose()
    {
        if (_handle.IsAllocated)
        {
            _handle.Free();
        }

        if (!_leaveOpen)
        {
            _stream.Dispose();
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static int ReadCallback(nint userData, byte* buffer, int bufferSize)
    {
        if (buffer is null || bufferSize <= 0 || !TryGetSource(userData, out ManagedStreamSource? source))
        {
            return InvalidOperation;
        }

        try
        {
            int bytesRead = source._stream.Read(new Span<byte>(buffer, bufferSize));
            return bytesRead == 0 ? EndOfFile : bytesRead;
        }
        catch
        {
            // Exceptions must never cross an unmanaged callback boundary.
            return InvalidOperation;
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static long SeekCallback(nint userData, long offset, int whence)
    {
        if (!TryGetSource(userData, out ManagedStreamSource? source) || !source._stream.CanSeek)
        {
            return InvalidOperation;
        }

        try
        {
            if ((whence & AvSeekSize) != 0)
            {
                return source._stream.Length;
            }

            int originValue = whence & ~(AvSeekSize | AvSeekForce);
            SeekOrigin origin = originValue switch
            {
                0 => SeekOrigin.Begin,
                1 => SeekOrigin.Current,
                2 => SeekOrigin.End,
                _ => throw new ArgumentOutOfRangeException(nameof(whence)),
            };

            return source._stream.Seek(offset, origin);
        }
        catch
        {
            return InvalidOperation;
        }
    }

    private static bool TryGetSource(
        nint userData,
        [NotNullWhen(true)] out ManagedStreamSource? source)
    {
        source = null;
        if (userData == 0)
        {
            return false;
        }

        try
        {
            source = GCHandle.FromIntPtr(userData).Target as ManagedStreamSource;
            return source is not null;
        }
        catch
        {
            return false;
        }
    }
}
