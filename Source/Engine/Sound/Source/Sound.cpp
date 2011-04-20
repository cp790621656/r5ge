#include "../Include/_All.h"
//#include <IrrKlang/Include/irrKlang.h>

//============================================================================================================
// Sound Library
//============================================================================================================

using namespace R5;

//#define SOURCE(source) ((irrklang::ISoundSource*)source)

Sound::~Sound()
{
	//mAudioSource = 0;
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
