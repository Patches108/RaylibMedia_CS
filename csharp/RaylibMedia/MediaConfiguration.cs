namespace RaylibMedia;

/// <summary>
/// Controls settings copied into subsequently loaded media streams. These settings are process-wide.
/// </summary>
public static class MediaConfiguration
{
    /// <summary>Gets a global native configuration value.</summary>
    public static int Get(MediaConfigurationFlag flag)
    {
        RaylibMediaRuntime.EnsureSupportedPlatform();
        int value = NativeMethods.GetMediaFlag((int)flag);
        if (value < 0)
        {
            throw new ArgumentOutOfRangeException(nameof(flag), flag, "Unknown media configuration flag.");
        }

        return value;
    }

    /// <summary>Sets a global value used by media streams loaded after this call.</summary>
    public static void Set(MediaConfigurationFlag flag, int value)
    {
        RaylibMediaRuntime.EnsureSupportedPlatform();
        if (NativeMethods.SetMediaFlag((int)flag, value) != 0)
        {
            throw new ArgumentOutOfRangeException(nameof(flag), flag, "Unknown media configuration flag.");
        }
    }

    /// <summary>Gets or sets the sample format used by subsequently loaded audio streams.</summary>
    public static MediaAudioFormat AudioFormat
    {
        get => (MediaAudioFormat)Get(MediaConfigurationFlag.AudioFormat);
        set
        {
            if (!Enum.IsDefined(value))
            {
                throw new ArgumentOutOfRangeException(nameof(value), value, "Unknown media audio format.");
            }

            Set(MediaConfigurationFlag.AudioFormat, (int)value);
        }
    }
}
