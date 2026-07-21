namespace RaylibMedia;

/// <summary>Thrown when FFmpeg cannot open or initialize a media source.</summary>
public sealed class MediaLoadException : Exception
{
    /// <summary>Creates an exception with the supplied failure description.</summary>
    public MediaLoadException(string message)
        : base(message)
    {
    }
}
