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
// Includes
//--------------------------------------------------------------------------------------------------

#include <raymedia.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>

//--------------------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------------------

#if defined(RAYLIB_VERSION_MAJOR) && defined(RAYLIB_VERSION_MINOR)
// Compatibility check for Raylib versions older than 5.5
#if (RAYLIB_VERSION_MAJOR < 5) || (RAYLIB_VERSION_MAJOR == 5 && RAYLIB_VERSION_MINOR < 5)
	#define IsTextureValid IsTextureReady
	#define IsModelValid IsModelReady
#endif
#else
	#error "RAYLIB_VERSION_MAJOR and RAYLIB_VERSION_MINOR must be defined"
#endif

#define VIDEO_CLIPS_COUNT (int)(sizeof(VIDEO_CLIPS) / sizeof(VIDEO_CLIPS[0]))

//--------------------------------------------------------------------------------------------------
// Constants and Enumerations
//--------------------------------------------------------------------------------------------------

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 540;
const char* EXAMPLE_TITLE = "Example 03 - Multiple Streams";

const char* VIDEO_CLIPS[] = {
	"009.mp4", "010.mp4", "003.mp4", "004.mp4", "005.mp4",
	"006.mp4", "007.mp4", "008.mp4", "011.mp4",
	"002.mp4", "001.mp4", "007.mp4", "001.mp4"
};

enum
{
	ENV_MODEL_BACKGROUND = 0,
	ENV_MODEL_FOREGROUND,
	ENV_MODEL_SCREENS,
	ENV_MODEL_COUNT
};

//--------------------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------------------

typedef struct SceneCamera
{
	Vector3 position;
	float fovYRad;
	Matrix projection;
	Matrix baseModelView;
	Vector3 offset;
	Vector3 rotation;
} SceneCamera;

typedef struct SceneData
{
	SceneCamera camera;
	Model envModel[ENV_MODEL_COUNT];
	Vector3 tvScreenPos[VIDEO_CLIPS_COUNT];
	MediaStream medias[VIDEO_CLIPS_COUNT];
} SceneData;

typedef struct ComputeVolumeData
{
	float minDist;
	float maxDist;
	float falloffRate;
	Matrix mvp; // Model-View-Projection matrix
	Vector2 mousePos;
	Vector2 screenSize;
} ComputeVolumeData;

typedef struct ComputeVolumeResult
{
	float volumeFactor;
	Vector2 tvScreenPos;
} ComputeVolumeResult;

//--------------------------------------------------------------------------------------------------
// Global Scene Data
//--------------------------------------------------------------------------------------------------

SceneData Scene = {0};

//--------------------------------------------------------------------------------------------------
// Function Declarations
//--------------------------------------------------------------------------------------------------

SceneCamera SetupSceneCamera(Vector3 position, float fovYDeg);
void UpdateSceneCamera(SceneCamera* camera, bool updateModelView, bool updatePerspective);
void BeginSceneCamera(const SceneCamera* camera, Matrix* outModelView);
void EndSceneCamera(void);

bool LoadScene(void);
void UnloadScene(void);
Color ColorBlend(Color color1, Color color2, float factor);
void RenderScene(void);
bool LoadEnvironment(void);
void UnloadEnvironment(void);

void OnWindowResized(void);
void HandleCamera(void);

ComputeVolumeResult ComputeVolume(Vector3 audioSourcePos, const ComputeVolumeData* cvd);

//--------------------------------------------------------------------------------------------------
// Main Entry Point
//--------------------------------------------------------------------------------------------------

int main(void)
{
	// Initialization
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TextFormat("raylib-media | %s", EXAMPLE_TITLE));
	InitAudioDevice();
	SetTargetFPS(60);

	// Enable depth test and depth mask
	rlEnableDepthTest();
	rlEnableDepthMask();

	if (!LoadScene())
	{
		TraceLog(LOG_ERROR, "Cannot load the scene.");
		CloseAudioDevice();
		CloseWindow();
		return -1;
	}

	// Main loop
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(BLACK);
		RenderScene();
		EndDrawing();
	}

	// De-initialization
	UnloadScene();
	CloseAudioDevice();
	CloseWindow();

	return 0;
}

//--------------------------------------------------------------------------------------------------
// Camera Functions
//--------------------------------------------------------------------------------------------------

SceneCamera SetupSceneCamera(Vector3 position, float fovYDeg)
{
	SceneCamera camera = {0};
	camera.position = position;
	camera.fovYRad = fovYDeg * DEG2RAD;
	camera.offset = (Vector3){0};
	camera.rotation = (Vector3){0};
	UpdateSceneCamera(&camera, true, true);
	return camera;
}

void UpdateSceneCamera(SceneCamera* camera, bool updateModelView, bool updatePerspective)
{
	if (updateModelView)
	{
		camera->baseModelView = MatrixLookAt(camera->position, (Vector3){camera->position.x, camera->position.y, 0},
		                                     (Vector3){0, 1, 0});
	}

	if (updatePerspective)
	{
		camera->projection = MatrixPerspective(camera->fovYRad, (float)(GetScreenWidth()) / (float)(GetScreenHeight()),
		                                       0.1f, 1000.0f);
	}
}

void BeginSceneCamera(const SceneCamera* camera, Matrix* outModelView)
{
	rlMatrixMode(RL_PROJECTION);
	rlPushMatrix();
	rlSetMatrixProjection(camera->projection);

	// Apply rotation and offset to the base model view matrix
	Matrix rotationMatrix = MatrixRotateXYZ(camera->rotation);
	Matrix translationMatrix = MatrixTranslate(camera->offset.x, camera->offset.y, camera->offset.z);
	*outModelView = MatrixMultiply(MatrixMultiply(rotationMatrix, camera->baseModelView), translationMatrix);

	rlMatrixMode(RL_MODELVIEW);
	rlLoadIdentity();
	rlSetMatrixModelview(*outModelView);
}

void EndSceneCamera(void)
{
	rlDrawRenderBatchActive();
	rlMatrixMode(RL_PROJECTION);
	rlPopMatrix();
	rlMatrixMode(RL_MODELVIEW);
	rlLoadIdentity();
}

//--------------------------------------------------------------------------------------------------
// Scene Functions
//--------------------------------------------------------------------------------------------------
bool LoadScene(void)
{
	Scene = (SceneData){0};

	if (!LoadEnvironment())
	{
		TraceLog(LOG_ERROR, "Failed to load environment.");
		return false;
	}

	Scene.camera = SetupSceneCamera((Vector3){0.39f, 1.75f, 8.26f}, 40.8f);

	return true;
}

void UnloadScene(void)
{
	UnloadEnvironment();
}

//--------------------------------------------------------------------------------------------------
// Environment Functions
//--------------------------------------------------------------------------------------------------
bool LoadEnvironment(void)
{
	// Load environment texture
	Texture envTexture = LoadTexture("resources/textures/tv_shop_env_texture_rgba.png");
	if (!IsTextureValid(envTexture))
	{
		TraceLog(LOG_ERROR, "Failed to load environment texture.");
		return false;
	}

	SetTextureFilter(envTexture, TEXTURE_FILTER_BILINEAR);

	// Load models
	Scene.envModel[ENV_MODEL_BACKGROUND] = LoadModel("resources/models/tv_shop_model_bg.obj");
	Scene.envModel[ENV_MODEL_FOREGROUND] = LoadModel("resources/models/tv_shop_model_fg.obj");
	Scene.envModel[ENV_MODEL_SCREENS] = LoadModel("resources/models/tv_shop_screens_model.glb");

	// Check if models are loaded and assign texture
	for (int i = 0; i < ENV_MODEL_COUNT - 1; ++i)
	{
		// Exclude ENV_MODEL_SCREENS
		if (!IsModelValid(Scene.envModel[i]))
		{
			TraceLog(LOG_ERROR, "Failed to load environment model %d.", i);
			return false;
		}
		Scene.envModel[i].materials[0].maps[MATERIAL_MAP_ALBEDO].texture = envTexture;
	}

	// Load media streams
	for (int i = 0; i < VIDEO_CLIPS_COUNT; ++i)
	{
		Scene.medias[i] = LoadMediaEx(TextFormat("resources/clips/%s", VIDEO_CLIPS[i]), MEDIA_FLAG_LOOP);
		if (!IsMediaValid(Scene.medias[i]))
		{
			TraceLog(LOG_ERROR, "Failed to load media stream %s.", VIDEO_CLIPS[i]);
			return false;
		}
	}

	// Setup screen models with media textures
	Model* screensModel = &Scene.envModel[ENV_MODEL_SCREENS];
	if (!IsModelValid(*screensModel))
	{
		TraceLog(LOG_ERROR, "Failed to load screens model.");
		return false;
	}

	// Allocate one material per mesh (screen)
	screensModel->materialCount = screensModel->meshCount;
	screensModel->materials = RL_REALLOC(screensModel->materials, sizeof(Material) * screensModel->materialCount);

	for (int i = 0; i < screensModel->materialCount; ++i)
	{
		screensModel->materials[i] = LoadMaterialDefault();
		screensModel->meshMaterial[i] = i;
		screensModel->materials[i].maps[MATERIAL_MAP_ALBEDO].color = Fade(WHITE, 0.75f);

		if (i < VIDEO_CLIPS_COUNT)
		{
			// Assign video texture to the material
			screensModel->materials[i].maps[MATERIAL_MAP_ALBEDO].texture = Scene.medias[i].videoTexture;

			// Calculate the position of the TV screen in world space
			BoundingBox bb = GetMeshBoundingBox(screensModel->meshes[i]);
			Vector3 bbSize = Vector3Subtract(bb.max, bb.min);
			Scene.tvScreenPos[i] = Vector3Add(bb.min, Vector3Scale(bbSize, 0.5f));
		}
		else
		{
			// Set a default color for unused screens
			screensModel->materials[i].maps[MATERIAL_MAP_ALBEDO].color = Fade(BLACK, 0.2f);
		}
	}

	return true;
}

void UnloadEnvironment(void)
{
	// Unload environment texture
	Texture envTexture = Scene.envModel[ENV_MODEL_BACKGROUND].materials[0].maps[MATERIAL_MAP_ALBEDO].texture;

	if (IsTextureValid(envTexture))
	{
		UnloadTexture(envTexture);
	}

	// Unload models
	for (int i = 0; i < ENV_MODEL_COUNT; ++i)
	{
		if (IsModelValid(Scene.envModel[i]))
		{
			UnloadModel(Scene.envModel[i]);
		}
	}

	// Unload media streams
	for (int i = 0; i < VIDEO_CLIPS_COUNT; ++i)
	{
		if (IsMediaValid(Scene.medias[i]))
		{
			UnloadMedia(&Scene.medias[i]);
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Render Functions
//--------------------------------------------------------------------------------------------------

Color ColorBlend(Color color1, Color color2, float factor)
{
    Color color = { 0 };

    if (factor < 0.0f) factor = 0.0f;
    else if (factor > 1.0f) factor = 1.0f;

    color.r = (unsigned char)((1.0f - factor)*color1.r + factor*color2.r);
    color.g = (unsigned char)((1.0f - factor)*color1.g + factor*color2.g);
    color.b = (unsigned char)((1.0f - factor)*color1.b + factor*color2.b);
    color.a = (unsigned char)((1.0f - factor)*color1.a + factor*color2.a);

    return color;
}

// Function to project a 3D world space position to 2D screen space
// Same as GetWorldToScreen()
Vector2 ProjectToScreen(Vector3 worldPos, const Matrix* mvp, float screenWidth, float screenHeight)
{
	// Transform world position to clip space
	Vector4 clipSpacePos;

	clipSpacePos.x = mvp->m0 * worldPos.x + mvp->m4 * worldPos.y + mvp->m8 * worldPos.z + mvp->m12;
	clipSpacePos.y = mvp->m1 * worldPos.x + mvp->m5 * worldPos.y + mvp->m9 * worldPos.z + mvp->m13;
	clipSpacePos.w = mvp->m3 * worldPos.x + mvp->m7 * worldPos.y + mvp->m11 * worldPos.z + mvp->m15;

	// Handle case when w is zero
	if (FloatEquals(clipSpacePos.w, 0.0f)) clipSpacePos.w = 0.00001f;

	// Normalize device coordinates
	clipSpacePos.x /= clipSpacePos.w;
	clipSpacePos.y /= clipSpacePos.w;

	// Convert to screen space coordinates
	Vector2 screenPos;
	screenPos.x = (clipSpacePos.x * 0.5f + 0.5f) * screenWidth;
	screenPos.y = (1.0f - (clipSpacePos.y * 0.5f + 0.5f)) * screenHeight;

	return screenPos;
}

void OnWindowResized(void)
{
	float screenWidth = (float)(GetScreenWidth());
	float screenHeight = (float)(GetScreenHeight());

	const float maxAspectRatio = 1.8f;

	float aspectRatio = screenWidth / screenHeight;

	// Limit aspect ratio
	if (aspectRatio > maxAspectRatio)
	{
		SetWindowSize((int)screenWidth, (int)(screenWidth / maxAspectRatio));
	}

	UpdateSceneCamera(&Scene.camera, false, true);
}

void HandleCamera(void)
{
	// Update mouse interaction
	float dt = GetFrameTime();
	float time = (float)GetTime();
	Vector2 mouseDelta = GetMouseDelta();

	Vector3 offset = Scene.camera.offset;

	offset.x -= mouseDelta.x * 0.001f;
	offset.y += mouseDelta.y * 0.001f;
	offset.z += IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? dt : -dt;

	// Update camera offset with time-based oscillation
	offset.x += 0.001f * cosf(time * 0.15f);
	offset.y += 0.001f * cosf(time * 0.5f);
	offset.z += 0.002f * cosf(time * 0.25f);

	// Clamp camera offset values
	Scene.camera.offset.x = Clamp(offset.x, 0.1f, 0.4f);
	Scene.camera.offset.y = Clamp(offset.y, -0.3f, 0.1f);
	Scene.camera.offset.z = Clamp(offset.z, 0.15f, 3.5f);

	// Update camera rotation with time-based oscillation
	Scene.camera.rotation.x = 0.5f * DEG2RAD * cosf(time * 0.1f);
	Scene.camera.rotation.z = 0.25f * DEG2RAD * cosf(time * 0.15f);
	Scene.camera.rotation.y = 1.0f * DEG2RAD * cosf(time * 0.05f);
}

ComputeVolumeResult ComputeVolume(Vector3 audioSourcePos, const ComputeVolumeData* cvd)
{
	ComputeVolumeResult cvr;

	// Project 3D position to 2D screen space
	cvr.tvScreenPos = ProjectToScreen(audioSourcePos, &cvd->mvp, cvd->screenSize.x, cvd->screenSize.y);

	float dist = Vector2Distance(cvr.tvScreenPos, cvd->mousePos);

	// Calculate volume factor based on distance
	if (dist < cvd->minDist)
	{
		cvr.volumeFactor = 1.0f;
	}
	else
	{
		float normalizedDist = (dist - cvd->minDist) / (cvd->maxDist - cvd->minDist);
		cvr.volumeFactor = expf(cvd->falloffRate * normalizedDist);
	}

	return cvr;
}

void RenderScene(void)
{
	// Handle window resizing------------------------------
	if (IsWindowResized())
	{
		OnWindowResized();
	}

	// Handle camera movements and input-------------------
	HandleCamera();

	// Update media streams--------------------------------
	for (int i = 0; i < VIDEO_CLIPS_COUNT; ++i)
	{
		UpdateMedia(&Scene.medias[i]);
	}

	// Begin 3D drawing------------------------------------
	Matrix modelView;
	BeginSceneCamera(&Scene.camera, &modelView);

	// Draw environment models
	for (int i = 0; i < ENV_MODEL_COUNT; ++i)
	{
		DrawModel(Scene.envModel[i], Vector3Zero(), 1.0f, WHITE);
	}

	// Draw screen "glow" effect
	BeginBlendMode(BLEND_ADDITIVE);
	DrawModel(Scene.envModel[ENV_MODEL_SCREENS], Vector3Zero(), 1.0f, Fade(WHITE, 0.25f));
	EndBlendMode();

	EndSceneCamera(); //-----------------------------------

	// Begin 2D drawing
	rlDisableDepthMask();
	rlDisableDepthTest();

	// Prepare data for volume computation
	ComputeVolumeData cvd;
	cvd.screenSize = (Vector2){(float)(GetScreenWidth()), (float)(GetScreenHeight())};
	cvd.maxDist = 0.15f * fmaxf(cvd.screenSize.x, cvd.screenSize.y);
	cvd.minDist = 12.0f;
	cvd.falloffRate = -3.0f;
	cvd.mvp = MatrixMultiply(modelView, Scene.camera.projection);
	cvd.mousePos = GetMousePosition();

	// Update audio volumes based on mouse position
	for (int i = 0; i < VIDEO_CLIPS_COUNT; ++i)
	{
		// Compute the volume
		ComputeVolumeResult cvr = ComputeVolume(Scene.tvScreenPos[i], &cvd);

		// Update the stream volume
		SetAudioStreamVolume(Scene.medias[i].audioStream, cvr.volumeFactor);

		// Draw visual feedback overlay
		float overlayAlpha = 4.0f * cvr.volumeFactor;
		DrawCircleLinesV(cvd.mousePos, cvd.maxDist, WHITE);
		DrawCircleV(cvr.tvScreenPos, 23.0f, Fade(BLACK, fminf(overlayAlpha, 0.55f)));
		DrawText(TextFormat("%0.2f", cvr.volumeFactor), (int)cvr.tvScreenPos.x - 18, (int)cvr.tvScreenPos.y - 8, 20,
		         ColorBlend((Color){255, 0, 0, 0}, GREEN, overlayAlpha));
	}

	// Draw Instructions overlay
	DrawRectangleRounded((Rectangle){10, 5, 330.0f, 70.0f}, 0.1f, 3, Fade(BLACK, 0.7f));
	DrawText("Left Mouse Button: Zoom In", 25, 15, 20, Fade(WHITE, 0.9f));
	DrawText("Mouse Position: Spatial Audio", 25, 45, 20, Fade(WHITE, 0.9f));

	DrawFPS((int)cvd.screenSize.x - 100,  10);

	// End 2D drawing
	rlDrawRenderBatchActive();
	rlEnableDepthMask();
	rlEnableDepthTest();
}
