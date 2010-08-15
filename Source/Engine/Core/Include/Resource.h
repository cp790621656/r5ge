#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Resource is just a way to pass strings to a worker thread
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