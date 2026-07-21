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

// This example includes icons from Google Material Icons (Apache License 2.0).
// "See resources/icons/LICENSE" for license details.

//--------------------------------------------------------------------------------------------------

#include <math.h>
#include <stddef.h>

#include "raymedia.h"

//--------------------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------------------

#if defined(RAYLIB_VERSION_MAJOR) && defined(RAYLIB_VERSION_MINOR)
// Compatibility check for Raylib versions older than 5.5
#if (RAYLIB_VERSION_MAJOR < 5) || (RAYLIB_VERSION_MAJOR == 5 && RAYLIB_VERSION_MINOR < 5)
#define IsTextureValid IsTextureReady
#define IsShaderValid IsShaderReady
#endif
#else
#error "RAYLIB_VERSION_MAJOR and RAYLIB_VERSION_MINOR must be defined"
#endif

//--------------------------------------------------------------------------------------------------

// Constants
const char* EXAMPLE_TITLE = "Example 02 - A Simple Media Player";
const int   SCREEN_WIDTH = 1280;
const int   SCREEN_HEIGHT = 720;
const char* MOVIE_FILE = "resources/videos/trailer.mp4"; // Adjust path to your movie file (e.g., "path/to/your_file.mp4")

// UI Metrics
const float BUTTON_SIZE = 40.0f;
const float BUTTON_PADDING = 5.0f;
const float MOVIE_BORDER_SIZE = 20.0f;
const float PROGRESS_BAR_MARGIN = 10.0f;
const float PROGRESS_BAR_BK_HEIGHT = 12.0f;
const float PROGRESS_BAR_FG_HEIGHT = 4.0f;

// UI Colors
const Color BG_COLOR_LIGHT = { 113, 0, 71, 255 };
const Color BG_COLOR_DARK = { 68, 0, 42, 255 };
const Color ICON_COLOR_BG = { 34, 0, 21, 210 };
const Color ICON_COLOR_DEFAULT = { 255, 0, 112, 255 };
const Color ICON_COLOR_HOVER = { 255, 255, 255, 255 };
const Color ICON_COLOR_SELECTED = { 254, 249, 0, 255 };
const Color ICON_COLOR_PRESSED = { 0, 228, 48, 255 };

// Enumerations
typedef enum {
    BTN_STATE_NORMAL = 0,
    BTN_STATE_HOVER,
    BTN_STATE_PRESSED,
    BTN_STATE_RELEASED
} ButtonState;

typedef enum {
    BTN_UNCHECKED = 0,
    BTN_CHECKED
} ButtonToggle;

typedef enum {
    ICO_PLAY = 0,
    ICO_PAUSE,
    ICO_FAST_FORWARD,
    ICO_LOOP,
    ICO_FAST_REWIND,
    ICO_VOLUME,
    ICO_BRIGHTNESS,
    ICO_CONTRAST,
    ICO_NO_SOUND,
    ICO_COLORS,
    ICO_BLUR,
    ICO_SPEED,

    ICON_COUNT
} ButtonIcon;

typedef enum {
    BTN_FAST_REWIND = 0,
    BTN_PLAY,
    BTN_FAST_FORWARD,
    BTN_SPEED,
    BTN_VOLUME,
    BTN_LOOP,
    BTN_GREYSCALE,
    BTN_PIXELATE,
    BTN_BLUR,

    BUTTON_COUNT
} ButtonId;

// Structures
typedef struct Button {
    ButtonId id;
    ButtonState state;
    ButtonToggle toggle;
    ButtonIcon icon;
    Color iconColor;
    Rectangle rect;
    const char* text;
} Button;

typedef struct GuiData {
    Texture2D icons[ICON_COUNT];
    Button buttons[BUTTON_COUNT];
    Vector2 offset;
} GuiData;

typedef struct ShaderData {
    Shader shader;
    int greyscaleLoc;
    int pixelateLoc;
    int blurLoc;
} ShaderData;

typedef struct PlayerData {
    GuiData gui;
    Rectangle dstRect;
    Rectangle srcRect;
    MediaProperties mediaProps;
    MediaStream media;
    ShaderData videoEffects;
} PlayerData;

//--------------------------------------------------------------------------------------------------

// Global player instance
PlayerData Player;

//--------------------------------------------------------------------------------------------------

// Function declarations
void PlayerSetPosition(MediaStream* media, double pos);
void PlayerShiftPosition(MediaStream* media, double shiftSeconds);
void PlayerSetLoop(bool enable);
void PlayerMute(bool enable);
void PlayerPause(bool enable);
bool LoadVideoEffects(void);
void UnloadVideoEffects(void);
Texture2D LoadIcon(const char* iconName);
bool LoadIcons(void);
void UnloadIcons(void);
Button BuildButton(ButtonId buttonId, ButtonIcon iconId, const char* text);
void BuildButtons(void);
Rectangle GetButtonRect(const Button* btn);
Rectangle InflateRect(Rectangle rect, float size);
void DrawButton(const Button* btn);
void HandleButton(Button* btn);
bool PlayerLoad(void);
void PlayerUnload(void);
void DrawProgressBar(void);
void OnWindowResized(void);

//--------------------------------------------------------------------------------------------------

// Main function
int main(void)
{
    // Initialization
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TextFormat("raylib-media | %s", EXAMPLE_TITLE));
    InitAudioDevice();

    SetTargetFPS(60);

    if (!PlayerLoad())
    {
        PlayerUnload();
        CloseAudioDevice();
        CloseWindow();
        return -1;
    }

    OnWindowResized();

    // Main loop
    while (!WindowShouldClose())
    {
        if (IsWindowResized())
        {
            OnWindowResized();
        }

        float frameTime = GetFrameTime();

        if (Player.gui.buttons[BTN_SPEED].toggle == BTN_CHECKED)
        {
            frameTime *= 4.0f;
        }

        UpdateMediaEx(&Player.media, frameTime);

        if (GetMediaState(Player.media) == MEDIA_STATE_STOPPED)
        {
            Player.gui.buttons[BTN_PLAY].icon = ICO_PLAY;
        }

        BeginDrawing();

        ClearBackground(BG_COLOR_LIGHT);
        DrawRectangleRec(InflateRect(Player.dstRect, MOVIE_BORDER_SIZE), BG_COLOR_DARK);

        BeginShaderMode(Player.videoEffects.shader);
        DrawTexturePro(Player.media.videoTexture, Player.srcRect, Player.dstRect, (Vector2) { 0, 0 }, 0.0f, WHITE);
        EndShaderMode();

        for (int i = 0; i < BUTTON_COUNT; ++i)
        {
            HandleButton(&Player.gui.buttons[i]);
        }
        DrawProgressBar();

        DrawFPS(GetScreenWidth() - 100, 5);

        EndDrawing();
    }

    PlayerUnload();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

//--------------------------------------------------------------------------------------------------

// Function implementations

void PlayerSetPosition(MediaStream* media, double pos)
{
    SetMediaPosition(*media, pos);

    // Force refresh video texture if paused
    if (GetMediaState(*media) == MEDIA_STATE_PAUSED)
    {
        SetMediaState(*media, MEDIA_STATE_PLAYING);
        UpdateMediaEx(media, 0.0f);
        SetMediaState(*media, MEDIA_STATE_PAUSED);
    }
}

void PlayerShiftPosition(MediaStream* media, double shiftSeconds)
{
    PlayerSetPosition(media, GetMediaPosition(*media) + shiftSeconds);
}

void PlayerSetLoop(bool enable)
{
    Player.gui.buttons[BTN_LOOP].toggle = enable ? BTN_CHECKED : BTN_UNCHECKED;
    SetMediaLooping(Player.media, enable);
}

void PlayerMute(bool enable)
{
    Button* btn = &Player.gui.buttons[BTN_VOLUME];
    if (enable)
    {
        btn->icon = ICO_NO_SOUND;
        btn->toggle = BTN_CHECKED;
        SetAudioStreamVolume(Player.media.audioStream, 0.0f);
    }
    else
    {
        btn->icon = ICO_VOLUME;
        btn->toggle = BTN_UNCHECKED;
        SetAudioStreamVolume(Player.media.audioStream, 1.0f);
    }
}

void PlayerPause(bool enable)
{
    Button* btn = &Player.gui.buttons[BTN_PLAY];
    if (enable)
    {
        btn->icon = ICO_PLAY;
        SetMediaState(Player.media, MEDIA_STATE_PAUSED);
    }
    else
    {
        btn->icon = ICO_PAUSE;
        SetMediaState(Player.media, MEDIA_STATE_PLAYING);
    }
}

bool LoadVideoEffects(void)
{
    ShaderData* sd = &Player.videoEffects;
    sd->shader = LoadShader(NULL, "resources/shaders/example_02.frag");
    if (IsShaderValid(sd->shader))
    {
        sd->greyscaleLoc = GetShaderLocation(sd->shader, "greyscale");
        sd->pixelateLoc = GetShaderLocation(sd->shader, "pixelate");
        sd->blurLoc = GetShaderLocation(sd->shader, "blur");
        return true;
    }
    TraceLog(LOG_ERROR, "LoadVideoEffects(): Failed loading shader.");
    return false;
}

void UnloadVideoEffects(void)
{
    ShaderData* sd = &Player.videoEffects;
    if (IsShaderValid(sd->shader))
    {
        UnloadShader(sd->shader);
    }
}

Texture2D LoadIcon(const char* iconName)
{
    Texture2D texture = LoadTexture(TextFormat("resources/icons/icon_%s.png", iconName));
    if (IsTextureValid(texture))
    {
        SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    }
    else
    {
        TraceLog(LOG_ERROR, "LoadIcon(): Failed to load icon '%s'", iconName);
    }
    return texture;
}

bool LoadIcons(void)
{
    Texture2D* icons = Player.gui.icons;
    icons[ICO_PLAY] = LoadIcon("play");
    icons[ICO_PAUSE] = LoadIcon("pause");
    icons[ICO_FAST_FORWARD] = LoadIcon("fast_forward");
    icons[ICO_SPEED] = LoadIcon("speed");
    icons[ICO_LOOP] = LoadIcon("loop");
    icons[ICO_FAST_REWIND] = LoadIcon("fast_rewind");
    icons[ICO_VOLUME] = LoadIcon("volume");
    icons[ICO_NO_SOUND] = LoadIcon("no_sound");
    icons[ICO_BRIGHTNESS] = LoadIcon("brightness");
    icons[ICO_CONTRAST] = LoadIcon("contrast");
    icons[ICO_COLORS] = LoadIcon("saturation");
    icons[ICO_BLUR] = LoadIcon("blur");

    for (int i = 0; i < ICON_COUNT; ++i)
    {
        if (!IsTextureValid(icons[i]))
        {
            TraceLog(LOG_ERROR, "LoadIcons(): Failed loading icon #%i", i);
            return false;
        }
    }
    return true;
}

void UnloadIcons(void)
{
    Texture2D* icons = Player.gui.icons;
    for (int i = 0; i < ICON_COUNT; ++i)
    {
        if (IsTextureValid(icons[i]))
        {
            UnloadTexture(icons[i]);
        }
    }
}

Button BuildButton(ButtonId buttonId, ButtonIcon iconId, const char* text)
{
    Button btn = { 0 };
    btn.id = buttonId;
    btn.rect = (Rectangle){ (BUTTON_SIZE + BUTTON_PADDING) * (float)buttonId, 0, BUTTON_SIZE, BUTTON_SIZE };
    btn.icon = iconId;
    btn.iconColor = ICON_COLOR_DEFAULT;
    btn.toggle = BTN_UNCHECKED;
    btn.state = BTN_STATE_NORMAL;
    btn.text = text;
    return btn;
}

void BuildButtons(void)
{
    Button* buttons = Player.gui.buttons;
    buttons[BTN_FAST_REWIND] = BuildButton(BTN_FAST_REWIND, ICO_FAST_REWIND, "-15s");
    buttons[BTN_PLAY] = BuildButton(BTN_PLAY, ICO_PAUSE, "Play/Pause");
    buttons[BTN_FAST_FORWARD] = BuildButton(BTN_FAST_FORWARD, ICO_FAST_FORWARD, "+15s");
    buttons[BTN_SPEED] = BuildButton(BTN_SPEED, ICO_SPEED, "Speed x4");
    buttons[BTN_VOLUME] = BuildButton(BTN_VOLUME, ICO_VOLUME, "Sound/Mute");
    buttons[BTN_LOOP] = BuildButton(BTN_LOOP, ICO_LOOP, "Loop Toggle");
    buttons[BTN_GREYSCALE] = BuildButton(BTN_GREYSCALE, ICO_CONTRAST, "Greyscale Toggle");
    buttons[BTN_PIXELATE] = BuildButton(BTN_PIXELATE, ICO_COLORS, "Pixelate Toggle");
    buttons[BTN_BLUR] = BuildButton(BTN_BLUR, ICO_BLUR, "Blur Toggle");
}

Rectangle GetButtonRect(const Button* btn)
{
    return (Rectangle) {
        btn->rect.x + Player.gui.offset.x,
            btn->rect.y + Player.gui.offset.y,
            btn->rect.width,
            btn->rect.height
    };
}

Rectangle InflateRect(Rectangle rect, float size)
{
    return (Rectangle) {
        rect.x - size,
            rect.y - size,
            rect.width + size * 2.0f,
            rect.height + size * 2.0f
    };
}

void DrawButton(const Button* btn)
{
    Rectangle btnRect = GetButtonRect(btn);
    Vector2 position = { btnRect.x, btnRect.y };
    Texture2D icon = Player.gui.icons[btn->icon];

    DrawRectangleRounded(btnRect, 0.15f, 3, ICON_COLOR_BG);
    DrawTextureEx(icon, position, 0.0f, BUTTON_SIZE / (float)icon.width, btn->iconColor);
}

void HandleButton(Button* btn)
{
    Rectangle btnRect = GetButtonRect(btn);

    bool isMouseInside = CheckCollisionPointRec(GetMousePosition(), btnRect);
    bool isMouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    if (isMouseInside)
    {
        if (isMouseDown)
        {
            btn->state = BTN_STATE_PRESSED;
        }
        else
        {
            if (btn->state == BTN_STATE_PRESSED)
            {
                btn->state = BTN_STATE_RELEASED;
            }
            else
            {
                btn->state = BTN_STATE_HOVER;
            }
        }
    }
    else
    {
        btn->state = BTN_STATE_NORMAL;
    }

    // Handle button action on release
    if (btn->state == BTN_STATE_RELEASED)
    {
        switch (btn->id)
        {
        case BTN_FAST_REWIND:
            PlayerShiftPosition(&Player.media, -15.0);
            break;
        case BTN_FAST_FORWARD:
            PlayerShiftPosition(&Player.media, +15.0);
            break;
        case BTN_PLAY:
            PlayerPause(btn->icon == ICO_PAUSE);
            break;
        case BTN_VOLUME:
            PlayerMute(btn->toggle == BTN_UNCHECKED);
            break;
        case BTN_LOOP:
            PlayerSetLoop(btn->toggle == BTN_UNCHECKED);
            break;
        case BTN_GREYSCALE:
            btn->toggle = btn->toggle == BTN_UNCHECKED ? BTN_CHECKED : BTN_UNCHECKED;
            SetShaderValue(Player.videoEffects.shader, Player.videoEffects.greyscaleLoc, &btn->toggle, SHADER_UNIFORM_INT);
            break;
        case BTN_PIXELATE:
            btn->toggle = btn->toggle == BTN_UNCHECKED ? BTN_CHECKED : BTN_UNCHECKED;
            SetShaderValue(Player.videoEffects.shader, Player.videoEffects.pixelateLoc, &btn->toggle, SHADER_UNIFORM_INT);
            break;
        case BTN_BLUR:
            btn->toggle = btn->toggle == BTN_UNCHECKED ? BTN_CHECKED : BTN_UNCHECKED;
            SetShaderValue(Player.videoEffects.shader, Player.videoEffects.blurLoc, &btn->toggle, SHADER_UNIFORM_INT);
            break;
        case BTN_SPEED:
            btn->toggle = btn->toggle == BTN_UNCHECKED ? BTN_CHECKED : BTN_UNCHECKED;
            break;
        default:
            break;
        }
    }

    // Update visual appearance based on state
    switch (btn->state)
    {
    case BTN_STATE_NORMAL:
        btn->iconColor = btn->toggle == BTN_UNCHECKED ? ICON_COLOR_DEFAULT : ICON_COLOR_SELECTED;
        break;
    case BTN_STATE_HOVER:
        btn->iconColor = btn->toggle == BTN_UNCHECKED ? ICON_COLOR_HOVER : ICON_COLOR_SELECTED;
        if (btn->text)
        {
            DrawRectangle(0, 0, GetScreenWidth(), 42, Fade(BG_COLOR_DARK, 0.8f));
            DrawText(btn->text, 30, 7, 30, WHITE);
        }
        break;
    case BTN_STATE_PRESSED:
        btn->iconColor = ICON_COLOR_PRESSED;
        break;
    case BTN_STATE_RELEASED:
        // Do nothing
        break;
    }

    // Draw button
    DrawButton(btn);
}

bool PlayerLoad(void)
{
    Player.media = LoadMedia(MOVIE_FILE);

    if (!IsMediaValid(Player.media))
    {
        TraceLog(LOG_ERROR, "LoadMedia(): Failed loading '%s'", MOVIE_FILE);
        return false;
    }

    if (!LoadIcons())
    {
        UnloadMedia(&Player.media);
        return false;
    }

    if (!LoadVideoEffects())
    {
        UnloadIcons();
        UnloadMedia(&Player.media);
        return false;
    }

    BuildButtons();

    Player.mediaProps = GetMediaProperties(Player.media);

    PlayerSetLoop(true);

    return true;
}

void PlayerUnload(void)
{
    UnloadMedia(&Player.media);
    UnloadVideoEffects();
    UnloadIcons();
}

void DrawProgressBar(void)
{
    float barTotalWidth = BUTTON_COUNT * (BUTTON_SIZE + BUTTON_PADDING) - PROGRESS_BAR_MARGIN - BUTTON_PADDING;
    float progress = (float)(GetMediaPosition(Player.media) / Player.mediaProps.durationSec);
    float barWidth = progress * (barTotalWidth - PROGRESS_BAR_MARGIN);

    Rectangle barBackground = {
        Player.gui.offset.x + PROGRESS_BAR_MARGIN * 0.5f,
        Player.gui.offset.y - PROGRESS_BAR_BK_HEIGHT - PROGRESS_BAR_MARGIN * 0.5f,
        barTotalWidth,
        PROGRESS_BAR_BK_HEIGHT };

    Rectangle barForeground = {
        barBackground.x + PROGRESS_BAR_MARGIN * 0.5f,
        barBackground.y + (barBackground.height - PROGRESS_BAR_FG_HEIGHT) * 0.5f,
        barWidth,
        PROGRESS_BAR_FG_HEIGHT };

    DrawRectangleRounded(barBackground, 0.45f, 3, ICON_COLOR_BG);
    DrawRectangleRec(barForeground, ICON_COLOR_DEFAULT);

    double position = GetMediaPosition(Player.media);
    int minutes = (int)(position / 60.0);
    int seconds = (int)(position - 60.0 * minutes);
    DrawText(TextFormat("%02i:%02i", minutes, seconds), (int)barForeground.x, (int)barBackground.y - 20, 20, ICON_COLOR_BG);
}

void OnWindowResized(void)
{
	const float sw = (float)(GetScreenWidth());
	const float sh = (float)(GetScreenHeight());

	const float ratio = (float)Player.media.videoTexture.width / (float)Player.media.videoTexture.height;

	const float h = sw / ratio;
	const float w = sh * ratio;

    if (w / h > ratio)
    {
        Player.dstRect = (Rectangle){ 0, (sh - h) * 0.5f, sw, h };
    }
    else
    {
        Player.dstRect = (Rectangle){ (sw - w) * 0.5f, 0, w, sh };
    }

    Player.srcRect = (Rectangle){ 0, 0, (float)Player.media.videoTexture.width, (float)Player.media.videoTexture.height };

    Player.gui.offset = (Vector2){ (sw - BUTTON_COUNT * (BUTTON_SIZE + BUTTON_PADDING)) * 0.5f, sh - 100 };
}
