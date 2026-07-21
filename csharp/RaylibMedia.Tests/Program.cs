using RaylibMedia;

static void Assert(bool condition, string message)
{
    if (!condition)
    {
        throw new InvalidOperationException(message);
    }
}

Assert((int)MediaLoadFlags.NoAudio == 2, "NoAudio ABI value changed.");
Assert((int)MediaLoadFlags.NoVideo == 4, "NoVideo ABI value changed.");
Assert((int)MediaLoadFlags.Loop == 8, "Loop ABI value changed.");
Assert((int)MediaLoadFlags.NoAutoplay == 16, "NoAutoplay ABI value changed.");
Assert(RaylibMediaRuntime.IsSupportedPlatform, "The Windows x64 smoke test platform was not recognized.");
Assert(RaylibMediaRuntime.RequiredNativeLibraries.Contains("avcodec-61.dll"), "FFmpeg runtime requirements changed.");

Assert(MediaStream.TryLoad("ok.mp4", out MediaStream? media, MediaLoadFlags.Loop), "Stub media did not load.");
using (MediaStream loaded = media!)
{
    Assert(loaded.Looping, "Initial loop flag was not retained.");
    Assert(loaded.VideoTexture.Id == 42, "Texture2D was not marshalled correctly.");
    Assert(loaded.VideoTexture.Width == 640 && loaded.VideoTexture.Height == 360, "Texture dimensions were not marshalled correctly.");

    MediaProperties properties = loaded.Properties;
    Assert(properties.DurationSeconds == 12.5, "Duration was not marshalled correctly.");
    Assert(properties.AverageFramesPerSecond == 24.0f, "Frame rate was not marshalled correctly.");
    Assert(properties.HasVideo && properties.HasAudio, "Native bool fields were not marshalled correctly.");

    Assert(loaded.State == MediaState.Playing, "Initial state is incorrect.");
    loaded.Pause();
    Assert(loaded.State == MediaState.Paused, "Pause failed.");
    loaded.Play();
    Assert(loaded.Update(0.25), "Explicit update failed.");
    Assert(loaded.PositionSeconds == 0.25, "Explicit update did not advance playback.");
    Assert(loaded.TrySeek(3.5) && loaded.PositionSeconds == 3.5, "Seek failed.");
    loaded.Looping = false;
    Assert(!loaded.Looping, "Loop state did not update.");
}

Assert(!MediaStream.TryLoad("missing", out _), "TryLoad accepted an invalid native stream.");

try
{
    _ = MediaStream.Load("missing");
    throw new InvalidOperationException("Load did not throw for an invalid native stream.");
}
catch (MediaLoadException)
{
}

using MemoryStream input = new([1, 2, 3, 4]);
using (MediaStream custom = MediaStream.Load(input, leaveOpen: true))
{
    Assert(custom.VideoTexture.Id == 84, "Managed stream callbacks failed.");
}

Assert(input.CanRead, "leaveOpen did not preserve the managed stream.");

MediaConfiguration.Set(MediaConfigurationFlag.VideoQueue, 77);
Assert(MediaConfiguration.Get(MediaConfigurationFlag.VideoQueue) == 77, "Configuration round trip failed.");

Console.WriteLine("All RaylibMedia interop smoke tests passed.");
