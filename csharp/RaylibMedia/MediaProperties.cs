namespace RaylibMedia;

/// <summary>Describes the streams and timing information in a media source.</summary>
public readonly record struct MediaProperties(
    double DurationSeconds,
    float AverageFramesPerSecond,
    bool HasVideo,
    bool HasAudio);
