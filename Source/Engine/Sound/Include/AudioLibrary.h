#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Author: Michael Lyashenko
//============================================================================================================

class AudioLibrary : public Thread::Lockable
{
public:
	AudioLibrary();
	~AudioLibrary();
};
