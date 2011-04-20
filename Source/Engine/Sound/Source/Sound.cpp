#include "../Include/_All.h"
#include "../Include/AL/al.h"
#include "../Include/AL/alc.h"
using namespace R5;

//============================================================================================================
// Sound Library
//============================================================================================================

Sound::~Sound()
{
	alDeleteBuffers(1, &mBuffer);
}

//============================================================================================================
// Play the specified in 2D
//============================================================================================================

ISoundInstance* Sound::Play (uint layer, float fadeInTime, bool repeat)
{
	return mAudio->Instantiate (this, layer, fadeInTime, repeat);
}

//============================================================================================================
// Play the specified sound at the specified position
//============================================================================================================

ISoundInstance* Sound::Play (const Vector3f& position, uint layer , float fadeInTime, bool repeat)
{
	return mAudio->Instantiate (this, position, layer, fadeInTime, repeat);
}
