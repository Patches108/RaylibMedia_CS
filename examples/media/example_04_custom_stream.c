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

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

//--------------------------------------------------------------------------------------------------

const char* ExampleTitle = "Example 04 - Custom Streams";

const int   ScreenWidth = 1920 / 2;
const int   ScreenHeight = 1080 / 2;
const char* MovieFile = "resources/clips/004.mp4"; // Adjust path to your movie file (e.g., "path/to/your_file.mp4")

//--------------------------------------------------------------------------------------------------

// This is an example structure representing a memory buffer with a read position
typedef struct MemoryStream
{
    uint8_t* data;    // Pointer to the memory buffer
    int64_t size;     // Total size of the buffer in bytes
    int64_t pos;      // Current read position within the buffer
} MemoryStream;


// This custom read function reads data from a MemoryStream
// @param userData Pointer to the user-provided MemoryStream
// @param buf Pointer to the destination buffer where data will be read into
// @param bufSize Number of bytes to read
// @return The number of bytes successfully read or MEDIA_IO_EOF on end of stream
int memoryRead(void* userData, uint8_t* buf, int bufSize)

{
    MemoryStream* ctx = (MemoryStream*)userData;

    if (ctx->pos >= ctx->size) 
    {
        return MEDIA_IO_EOF; // End of file
    }

    int64_t bytesToRead = bufSize;

    if (ctx->pos + bytesToRead > ctx->size) 
    {
        bytesToRead = ctx->size - ctx->pos;
    }

    memcpy(buf, ctx->data + ctx->pos, bytesToRead);

    ctx->pos += bytesToRead;

    return (int)bytesToRead;
}

// This custom seek function modifies the read position in the MemoryStream
// @param userData Pointer to the user-provided MemoryStream
// @param offset Offset to apply based on whence
// @param whence Reference position for the offset (SEEK_SET, SEEK_CUR, SEEK_END)
// @return The new position in the stream or MEDIA_IO_INVALID on error
int64_t memorySeek(void* userData, int64_t offset, int whence)

{
    MemoryStream* ctx = (MemoryStream*)userData;

    int64_t newPos;

    switch (whence)
	{
    case SEEK_SET:
        newPos = offset;
        break;
    case SEEK_CUR:
        newPos = ctx->pos + offset;
        break;
    case SEEK_END:
        newPos = ctx->size + offset;
        break;
    default:
        return MEDIA_IO_INVALID;
    }

    if (newPos < 0 || newPos >(int64_t)ctx->size)
    {
        return MEDIA_IO_INVALID; // Invalid seek
    }

    ctx->pos = (int64_t)newPos;

    return newPos;
}

// Loads a MemoryStream by reading the entire contents of a file into a memory buffer
// @param fileName Path to the file to load into memory
// @return A MemoryStream with the file contents; empty on failure
MemoryStream LoadMemoryStream(const char* fileName)

{
    MemoryStream retStream = { 0 };

    FILE* file = fopen(fileName, "rb"); 
    if (file == NULL) 
    {
        TraceLog(LOG_ERROR, "MemoryStream: Error opening file");
        return retStream;
    }

    // Retrieving the file size by seeking to the end of the file and reading the position
    fseek(file, 0, SEEK_END);
    retStream.size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (retStream.size <= 0)
    {
        TraceLog(LOG_ERROR, "MemoryStream: Error getting file size");
        fclose(file);
        return retStream;
    }

    // Allocate memory to hold the file content
    retStream.data = RL_MALLOC(retStream.size);
	if (retStream.data == NULL) 
    {
        TraceLog(LOG_ERROR, "MemoryStream: Error allocating memory");
        fclose(file);
        return retStream;
    }

    // Read the whole file into the buffer
    const size_t bytesRead = fread(retStream.data, 1, retStream.size, file);
    if (bytesRead != (size_t)retStream.size)
    {
        TraceLog(LOG_ERROR, "MemoryStream: Error reading file");

    	RL_FREE(retStream.data);
        retStream.data = NULL;

        fclose(file);
        return retStream;
    }

    fclose(file); // Close the file

    return retStream;
}

// Frees the resources associated with a MemoryStream
// @param memoryStream Pointer to the MemoryStream to unload
void UnloadMemoryStream(MemoryStream* memoryStream)

{
	if (memoryStream->data)
	{
        RL_FREE(memoryStream->data);
        memoryStream->data = NULL;
	}
}

//--------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    // Setup: Initialize window, audio, and load media
    InitWindow(ScreenWidth, ScreenHeight, TextFormat("raylib-media | %s", ExampleTitle));
    SetTargetFPS(60);  // Set frame rate to 60 frames per second
    InitAudioDevice();

    // You can change the internal buffer used to read the custom stream by setting MEDIA_IO_BUFFER flag.
    // Default size is: 4096 bytes
    // The IO buffer size is very important for performance.
    //    * For protocols with fixed blocksize it should be set to this blocksize.
    //    * For others a typical size is a cache page, e.g., 4kb.

    // SetMediaFlag(MEDIA_IO_BUFFER, 4096);

	// Load the entire file into a memory buffer
    MemoryStream memoryStream = LoadMemoryStream(MovieFile);

    // Set up a MediaStreamReader with custom read and seek functions for the MemoryStream
    MediaStreamReader streamReader = { 0 };
    streamReader.readFn = &memoryRead;         // Custom read function for the memory stream
    streamReader.seekFn = &memorySeek;         // Custom seek function for the memory stream
    streamReader.userData = &memoryStream;     // User-defined context (MemoryStream)

    // Load the media from the memory stream with default settings
    MediaStream videoMedia = LoadMediaFromStream(streamReader, MEDIA_LOAD_AV);

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

    // Cleanup: Unload media and stream and close devices
    UnloadMedia(&videoMedia);
    CloseAudioDevice();
    UnloadMemoryStream(&memoryStream);
    CloseWindow();

    return 0;
}

//--------------------------------------------------------------------------------------------------