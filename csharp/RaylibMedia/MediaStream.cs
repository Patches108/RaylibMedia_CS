using System.Diagnostics.CodeAnalysis;
using Raylib_cs;

namespace RaylibMedia;

/// <summary>
/// Decodes a media source into a Raylib texture and audio stream.
/// </summary>
/// <remarks>
/// Initialize Raylib's window and audio device before loading media. Call <see cref="Update()"/>
/// once per game frame, and dispose the media before closing the audio device or window.
/// </remarks>
public sealed class MediaStream : IDisposable
{
    private NativeMethods.NativeMediaStream _native;
    private ManagedStreamSource? _managedSource;
    private bool _disposed;
    private bool _looping;

    private MediaStream(
        NativeMethods.NativeMediaStream native,
        MediaLoadFlags flags,
        ManagedStreamSource? managedSource = null)
    {
        _native = native;
        _managedSource = managedSource;
        _looping = (flags & MediaLoadFlags.Loop) != 0;
    }

    /// <summary>Loads a path or URL through FFmpeg.</summary>
    /// <exception cref="MediaLoadException">The source could not be opened or initialized.</exception>
    public static MediaStream Load(string fileName, MediaLoadFlags flags = MediaLoadFlags.AudioVideo)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(fileName);
        RaylibMediaRuntime.EnsureSupportedPlatform();
        ValidateFlags(flags);

        NativeMethods.NativeMediaStream native = NativeMethods.LoadMedia(fileName, flags);
        if (!IsValidOrRelease(ref native))
        {
            throw new MediaLoadException($"Could not load media source '{fileName}'. See the Raylib log for FFmpeg details.");
        }

        return new MediaStream(native, flags);
    }

    /// <summary>Attempts to load a path or URL through FFmpeg.</summary>
    public static bool TryLoad(
        string fileName,
        [NotNullWhen(true)] out MediaStream? media,
        MediaLoadFlags flags = MediaLoadFlags.AudioVideo)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(fileName);
        RaylibMediaRuntime.EnsureSupportedPlatform();
        ValidateFlags(flags);

        NativeMethods.NativeMediaStream native = NativeMethods.LoadMedia(fileName, flags);
        if (!IsValidOrRelease(ref native))
        {
            media = null;
            return false;
        }

        media = new MediaStream(native, flags);
        return true;
    }

    /// <summary>
    /// Loads media using a managed stream. The stream is owned by the returned media unless
    /// <paramref name="leaveOpen"/> is <see langword="true"/>.
    /// </summary>
    public static MediaStream Load(
        Stream source,
        MediaLoadFlags flags = MediaLoadFlags.AudioVideo,
        bool leaveOpen = false)
    {
        ArgumentNullException.ThrowIfNull(source);
        RaylibMediaRuntime.EnsureSupportedPlatform();
        if (!source.CanRead)
        {
            throw new ArgumentException("The media stream must be readable.", nameof(source));
        }

        ValidateFlags(flags);
        ManagedStreamSource managedSource = new(source, leaveOpen);

        try
        {
            NativeMethods.NativeMediaStream native =
                NativeMethods.LoadMediaFromStream(managedSource.CreateReader(), (int)flags);

            if (!IsValidOrRelease(ref native))
            {
                throw new MediaLoadException("Could not load media from the managed stream. See the Raylib log for FFmpeg details.");
            }

            return new MediaStream(native, flags, managedSource);
        }
        catch
        {
            managedSource.Dispose();
            throw;
        }
    }

    /// <summary>The current decoded video frame. Only valid when <see cref="Properties"/> reports video.</summary>
    public Texture2D VideoTexture
    {
        get
        {
            ThrowIfDisposed();
            return _native.VideoTexture;
        }
    }

    /// <summary>The native Raylib audio stream. Only valid when <see cref="Properties"/> reports audio.</summary>
    public AudioStream AudioStream
    {
        get
        {
            ThrowIfDisposed();
            return _native.AudioStream;
        }
    }

    /// <summary>Gets the source duration, frame rate, and available streams.</summary>
    public MediaProperties Properties
    {
        get
        {
            ThrowIfDisposed();
            NativeMethods.NativeMediaProperties value = NativeMethods.GetMediaProperties(_native);
            return new MediaProperties(
                value.DurationSeconds,
                value.AverageFramesPerSecond,
                value.HasVideo,
                value.HasAudio);
        }
    }

    /// <summary>Gets the current playback state.</summary>
    public MediaState State
    {
        get
        {
            ThrowIfDisposed();
            return (MediaState)NativeMethods.GetMediaState(_native);
        }
    }

    /// <summary>Gets or seeks the playback position in seconds.</summary>
    public double PositionSeconds
    {
        get
        {
            ThrowIfDisposed();
            return NativeMethods.GetMediaPosition(_native);
        }
        set
        {
            if (!TrySeek(value))
            {
                throw new InvalidOperationException($"Could not seek the media stream to {value} seconds.");
            }
        }
    }

    /// <summary>Gets or sets whether playback restarts at the end of the source.</summary>
    public bool Looping
    {
        get
        {
            ThrowIfDisposed();
            return _looping;
        }
        set
        {
            ThrowIfDisposed();
            if (!NativeMethods.SetMediaLooping(_native, value))
            {
                throw new InvalidOperationException("Could not change media looping.");
            }

            _looping = value;
        }
    }

    /// <summary>Advances decoding using Raylib's frame time.</summary>
    public bool Update()
    {
        ThrowIfDisposed();
        return NativeMethods.UpdateMedia(ref _native);
    }

    /// <summary>Advances decoding by an explicit elapsed time in seconds.</summary>
    public bool Update(double deltaTime)
    {
        ThrowIfDisposed();
        if (!double.IsFinite(deltaTime) || deltaTime < 0)
        {
            throw new ArgumentOutOfRangeException(nameof(deltaTime), "Delta time must be finite and non-negative.");
        }

        return NativeMethods.UpdateMediaEx(ref _native, deltaTime);
    }

    /// <summary>Starts or resumes playback.</summary>
    public void Play() => SetState(MediaState.Playing);

    /// <summary>Pauses playback at the current position.</summary>
    public void Pause() => SetState(MediaState.Paused);

    /// <summary>Stops playback.</summary>
    public void Stop() => SetState(MediaState.Stopped);

    /// <summary>Attempts to seek to a time in seconds.</summary>
    public bool TrySeek(double timeSeconds)
    {
        ThrowIfDisposed();
        if (!double.IsFinite(timeSeconds) || timeSeconds < 0)
        {
            return false;
        }

        return NativeMethods.SetMediaPosition(_native, timeSeconds);
    }

    /// <summary>Unloads all native decoder, audio, and texture resources.</summary>
    public void Dispose()
    {
        if (_disposed)
        {
            return;
        }

        _disposed = true;

        try
        {
            if (_native.Context != 0)
            {
                NativeMethods.UnloadMedia(ref _native);
            }
        }
        finally
        {
            _managedSource?.Dispose();
            _managedSource = null;
        }
    }

    private void SetState(MediaState state)
    {
        ThrowIfDisposed();
        if (state is MediaState.Invalid)
        {
            throw new ArgumentOutOfRangeException(nameof(state));
        }

        MediaState result = (MediaState)NativeMethods.SetMediaState(_native, (int)state);
        if (result == MediaState.Invalid)
        {
            throw new InvalidOperationException($"Could not set media state to {state}.");
        }
    }

    private static void ValidateFlags(MediaLoadFlags flags)
    {
        const MediaLoadFlags knownFlags = MediaLoadFlags.NoAudio
            | MediaLoadFlags.NoVideo
            | MediaLoadFlags.Loop
            | MediaLoadFlags.NoAutoplay;

        if ((flags & ~knownFlags) != 0)
        {
            throw new ArgumentOutOfRangeException(nameof(flags), flags, "Unknown media load flag.");
        }

        if ((flags & (MediaLoadFlags.NoAudio | MediaLoadFlags.NoVideo)) ==
            (MediaLoadFlags.NoAudio | MediaLoadFlags.NoVideo))
        {
            throw new ArgumentException("Audio and video cannot both be disabled.", nameof(flags));
        }
    }

    private static bool IsValidOrRelease(ref NativeMethods.NativeMediaStream native)
    {
        if (NativeMethods.IsMediaValid(native))
        {
            return true;
        }

        if (native.Context != 0)
        {
            NativeMethods.UnloadMedia(ref native);
        }

        return false;
    }

    private void ThrowIfDisposed()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
    }
}
