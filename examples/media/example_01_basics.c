/***************************************************************************************************
*
*   LICENSE: zlib
*
*   Copyright (c) 2024 Claudio Z. (@cloudofoz)
*
*   This software is provided "as-is," without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
***************************************************************************************************/

//--------------------------------------------------------------------------------------------------

#include "raymedia.h"

//--------------------------------------------------------------------------------------------------

const char* ExampleTitle = "Example 01 - Basics";

const int   ScreenWidth = 1920 / 2;
const int   ScreenHeight = 1080 / 2;
const char* MovieFile = "resources/clips/005.mp4"; // Adjust path to your movie file (e.g., "path/to/your_file.mp4")

//--------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    // Setup: Initialize window, audio, and load media
    InitWindow(ScreenWidth, ScreenHeight, TextFormat("raylib-media | %s", ExampleTitle));
    SetTargetFPS(60);  // Set frame rate to 60 frames per second
    InitAudioDevice();

    // Load the media stream with default settings
    MediaStream videoMedia = LoadMedia(MovieFile);

    // Verify if the media has loaded correctly 
    if (!IsMediaValid(videoMedia))
    {
        TraceLog(LOG_ERROR, "Failed to load media file: %s", MovieFile);
        CloseAudioDevice();
        CloseWindow();
        return -1;
    }

    // Set the media to play in a continuous loop
    SetMediaLooping(videoMedia, true);

    // Main Loop: Update and draw the media
    while (!WindowShouldClose())
    {
        // Update the media stream based on frame timing
        UpdateMedia(&videoMedia);

        BeginDrawing();
        ClearBackground(DARKPURPLE);

        // Calculate the coordinates to center the video in the window
        const int videoPosX = (GetScreenWidth() - videoMedia.videoTexture.width) / 2;
        const int videoPosY = (GetScreenHeight() - videoMedia.videoTexture.height) / 2;

        // Draw the video stream texture at the calculated position
        DrawTexture(videoMedia.videoTexture, videoPosX, videoPosY, WHITE);

        EndDrawing();
    }

    // Cleanup: Unload media and close devices
    UnloadMedia(&videoMedia);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

//--------------------------------------------------------------------------------------------------