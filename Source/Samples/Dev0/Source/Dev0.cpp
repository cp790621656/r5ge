//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev0 is a temporary testing application. Its source code and purpose change frequently.
//============================================================================================================

// Include R5
#include "../../../Engine/Interface/Include/_All.h"

// For some strange reason FMOD doesn't link unless <windows.h> is included prior to its header files.
#include <windows.h>

// Include FMod
#include "../Include/fmod.hpp"
#include "../Include/fmod_errors.h"
#pragma comment(lib, "fmodex.lib")

using namespace R5;

//============================================================================================================
// FMod error check
//============================================================================================================

bool FMODCheck (FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		return false;
	}
	return true;
}

//============================================================================================================
// Application entry point
//============================================================================================================

int main (int argc, char* argv[])
{
#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif
	System::SetCurrentPath("../../../Resources/");

	printf("FMOD Sound System, copyright © Firelight Technologies Pty, Ltd., 1994-2010.\n");

	FMOD::System* system (0);
    FMOD_RESULT result = FMOD::System_Create(&system);
    if (!FMODCheck(result)) return 0;

	uint version (0);
    result = system->getVersion(&version);
    if (!FMODCheck(result)) return 0;

    if (version < FMOD_VERSION)
    {
        printf("Error! You are using an old version of FMOD %08x. This program requires %08x\n",
			version, FMOD_VERSION);
        getchar();
        return 0;
    }

    result = system->init(32, FMOD_INIT_NORMAL, NULL);
    if (!FMODCheck(result)) return 0;

	printf("Creating the sound... ");
	FMOD::Sound* sound = 0;
	result = system->createStream("Sound/cAudioTheme1.ogg", FMOD_LOOP_NORMAL, 0, &sound);
	printf(" done.\n");

	if (FMODCheck(result))
	{
		// Just for the fun of it, set a preset
		FMOD_REVERB_PROPERTIES prop = FMOD_PRESET_SEWERPIPE;
		system->setReverbProperties(&prop);

		// Play the sound
		FMOD::Channel* channel (0);
		result = system->playSound(FMOD_CHANNEL_FREE, sound, false, &channel);

		if (FMODCheck(result))
		{
			system->update();
			printf("Press Enter key to stop playback\n");
			getchar();
		}
	}

	result = system->release();
	if (!FMODCheck(result)) return 0;

	printf("Done! Press Enter to exit.\n");
	getchar();
	return 0;
}