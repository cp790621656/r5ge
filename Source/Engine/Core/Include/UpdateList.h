#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Container for registered update callbacks
// TODO: Is it now obsolete with the addition of scripts?
//============================================================================================================

class UpdateList
{
public:

	// The return value has the following meaning:
	// Return value of < 0 means the callback should be removed and won't be called again.
	// Any other return value is added to the current time, and the callback will be triggered
	// again when the Time::GetTime() reaches that mark.

	typedef FastDelegate<float (void)> Callback;

private:

	struct UpdateEntry
	{
		float		time;
		Callback	callback;
	};

	Array<UpdateEntry>		mUpdate;

public:

	// Adds a new update function to the list, executed after an optional delay in seconds
	void Add (const Callback& callback, float delay = 0.0f);

	// Runs through all update listeners and calls them as necessary
	void Execute();
};