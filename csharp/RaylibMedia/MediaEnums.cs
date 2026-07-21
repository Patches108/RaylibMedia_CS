namespace RaylibMedia;

/// <summary>Options used while opening a media source.</summary>
[Flags]
public enum MediaLoadFlags
{
    /// <summary>Load both audio and video and begin playback immediately.</summary>
    AudioVideo = 0,

    /// <summary>Do not load the audio stream.</summary>
    NoAudio = 1 << 1,

    /// <summary>Do not load the video stream.</summary>
    NoVideo = 1 << 2,

    /// <summary>Restart playback when the end of the source is reached.</summary>
    Loop = 1 << 3,

    /// <summary>Open the source in the paused state.</summary>
    NoAutoplay = 1 << 4,
}

/// <summary>The current playback state.</summary>
public enum MediaState
{
    /// <summary>The native stream is absent or invalid.</summary>
    Invalid = -1,

    /// <summary>Playback is stopped.</summary>
    Stopped = 0,

    /// <summary>Playback is paused.</summary>
    Paused = 1,

    /// <summary>Playback is running.</summary>
    Playing = 2,
}

/// <summary>A global raylib-media configuration setting.</summary>
public enum MediaConfigurationFlag
{
    /// <summary>Custom input buffer capacity in bytes.</summary>
    IoBuffer = 0,

    /// <summary>Pending video packet capacity.</summary>
    VideoQueue = 1,

    /// <summary>Pending audio packet capacity.</summary>
    AudioQueue = 2,

    /// <summary>Decoded audio buffer capacity in bytes.</summary>
    AudioDecodedBuffer = 3,

    /// <summary>Raylib audio stream buffer size.</summary>
    AudioStreamBuffer = 4,

    /// <summary>Output <see cref="MediaAudioFormat"/> value.</summary>
    AudioFormat = 5,

    /// <summary>Output channel count.</summary>
    AudioChannels = 6,

    /// <summary>Maximum video delay before dropping a packet, in milliseconds.</summary>
    VideoMaximumDelay = 7,

    /// <summary>Maximum audio delay before dropping a packet, in milliseconds.</summary>
    AudioMaximumDelay = 8,

    /// <summary>Maximum audio bytes uploaded to Raylib per frame.</summary>
    AudioUpdate = 9,
}

/// <summary>The sample format produced by a media audio stream.</summary>
public enum MediaAudioFormat
{
    /// <summary>Unsigned 8-bit integer samples.</summary>
    Unsigned8Bit = 0,

    /// <summary>Signed 16-bit integer samples (the default).</summary>
    Signed16Bit = 1,

    /// <summary>Signed 32-bit integer samples.</summary>
    Signed32Bit = 2,

    /// <summary>32-bit floating-point samples.</summary>
    Float = 3,

    /// <summary>64-bit floating-point samples.</summary>
    Double = 4,
}
