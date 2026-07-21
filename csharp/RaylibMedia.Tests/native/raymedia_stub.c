#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if defined(_WIN32)
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif

typedef struct Texture {
    unsigned int id;
    int width;
    int height;
    int mipmaps;
    int format;
} Texture;

typedef struct AudioStream {
    void *buffer;
    void *processor;
    unsigned int sampleRate;
    unsigned int sampleSize;
    unsigned int channels;
} AudioStream;

typedef struct MediaStream {
    Texture videoTexture;
    AudioStream audioStream;
    void *ctx;
} MediaStream;

typedef struct MediaProperties {
    double durationSec;
    float avgFPS;
    bool hasVideo;
    bool hasAudio;
} MediaProperties;

typedef int (*ReadCallback)(void *userData, uint8_t *buffer, int bufferSize);
typedef int64_t (*SeekCallback)(void *userData, int64_t offset, int whence);

typedef struct MediaStreamReader {
    ReadCallback readFn;
    SeekCallback seekFn;
    void *userData;
} MediaStreamReader;

typedef struct StubContext {
    int state;
    double position;
    bool looping;
} StubContext;

static StubContext fileContext;
static StubContext streamContext;
static int flags[10];

static MediaStream CreateMedia(StubContext *context, unsigned int textureId, int loadFlags) {
    MediaStream media = { 0 };
    context->state = (loadFlags & 16) ? 1 : 2;
    context->position = 0;
    context->looping = (loadFlags & 8) != 0;
    media.videoTexture = (Texture) { textureId, 640, 360, 1, 4 };
    media.audioStream.sampleRate = 48000;
    media.audioStream.sampleSize = 16;
    media.audioStream.channels = 2;
    media.ctx = context;
    return media;
}

EXPORT MediaStream LoadMediaEx(const char *fileName, int loadFlags) {
    if (!fileName || strcmp(fileName, "missing") == 0) return (MediaStream) { 0 };
    return CreateMedia(&fileContext, 42, loadFlags);
}

EXPORT MediaStream LoadMediaFromStream(MediaStreamReader reader, int loadFlags) {
    uint8_t data[4] = { 0 };
    if (!reader.readFn || reader.readFn(reader.userData, data, 4) != 4) return (MediaStream) { 0 };
    if (reader.seekFn && reader.seekFn(reader.userData, 0, 0x10000) != 4) return (MediaStream) { 0 };
    if (memcmp(data, "\x01\x02\x03\x04", 4) != 0) return (MediaStream) { 0 };
    return CreateMedia(&streamContext, 84, loadFlags);
}

EXPORT bool IsMediaValid(MediaStream media) {
    return media.ctx != 0;
}

EXPORT MediaProperties GetMediaProperties(MediaStream media) {
    MediaProperties properties = { 0 };
    if (media.ctx) properties = (MediaProperties) { 12.5, 24.0f, true, true };
    return properties;
}

EXPORT bool UpdateMedia(MediaStream *media) {
    return media && media->ctx;
}

EXPORT bool UpdateMediaEx(MediaStream *media, double deltaTime) {
    if (!media || !media->ctx) return false;
    ((StubContext *)media->ctx)->position += deltaTime;
    return true;
}

EXPORT int GetMediaState(MediaStream media) {
    return media.ctx ? ((StubContext *)media.ctx)->state : -1;
}

EXPORT int SetMediaState(MediaStream media, int newState) {
    if (!media.ctx) return -1;
    ((StubContext *)media.ctx)->state = newState;
    return newState;
}

EXPORT double GetMediaPosition(MediaStream media) {
    return media.ctx ? ((StubContext *)media.ctx)->position : -1;
}

EXPORT bool SetMediaPosition(MediaStream media, double timeSeconds) {
    if (!media.ctx || timeSeconds < 0) return false;
    ((StubContext *)media.ctx)->position = timeSeconds;
    return true;
}

EXPORT bool SetMediaLooping(MediaStream media, bool loop) {
    if (!media.ctx) return false;
    ((StubContext *)media.ctx)->looping = loop;
    return true;
}

EXPORT int SetMediaFlag(int flag, int value) {
    if (flag < 0 || flag >= 10) return -1;
    flags[flag] = value;
    return 0;
}

EXPORT int GetMediaFlag(int flag) {
    return (flag < 0 || flag >= 10) ? -1 : flags[flag];
}

EXPORT void UnloadMedia(MediaStream *media) {
    if (media) *media = (MediaStream) { 0 };
}
