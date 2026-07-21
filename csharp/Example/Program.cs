using Raylib_cs;
using RaylibMedia;

if (args.Length != 1)
{
    Console.Error.WriteLine("Usage: dotnet run --project csharp/Example -- <video-file>");
    return 1;
}

Raylib.InitWindow(960, 540, "raylib-media C# example");
Raylib.SetTargetFPS(60);
Raylib.InitAudioDevice();

try
{
    using MediaStream media = MediaStream.Load(args[0], MediaLoadFlags.Loop);
    if (media.Properties.HasAudio)
    {
        Raylib.SetAudioStreamVolume(media.AudioStream, 1.0f);
    }

    while (!Raylib.WindowShouldClose())
    {
        media.Update();

        Texture2D texture = media.VideoTexture;
        int x = (Raylib.GetScreenWidth() - texture.Width) / 2;
        int y = (Raylib.GetScreenHeight() - texture.Height) / 2;

        Raylib.BeginDrawing();
        Raylib.ClearBackground(Color.DarkPurple);
        Raylib.DrawTexture(texture, x, y, Color.White);
        Raylib.EndDrawing();
    }
}
finally
{
    Raylib.CloseAudioDevice();
    Raylib.CloseWindow();
}

return 0;
