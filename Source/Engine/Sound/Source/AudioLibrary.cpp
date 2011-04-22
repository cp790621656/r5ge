#include "../Include/_All.h"
#include "../Include/AL/al.h"
#include "../Include/AL/alc.h"
using namespace R5;

#ifdef _WINDOWS

#pragma comment(lib, "ogg.lib")
#pragma comment(lib, "vorbis.lib")
#pragma comment(lib, "OpenAL32.lib")

// NOTE: This part is only necessary if OpenAL is a static library
AudioLibrary::AudioLibrary() { alcInit(); }
AudioLibrary::~AudioLibrary() { alcRelease(); }

#else

AudioLibrary::AudioLibrary() {}
AudioLibrary::~AudioLibrary() {}

#endif