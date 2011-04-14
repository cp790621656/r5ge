#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Resource is just a way to pass strings to a worker thread
// Author: Michael Lyashenko
//============================================================================================================

class Core;
class Resource
{
protected:

	String	mName;
	Core*	mCore;

	friend class Core;

public:

	Resource(const String& name) : mName(name) {}

	R5_DECLARE_SOLO_CLASS("Resource");

	Core* GetCore() { return mCore; }

	const String& GetName() const { return mName; }
};