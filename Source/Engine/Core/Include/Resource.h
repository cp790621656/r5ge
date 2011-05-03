#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
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

	R5_DECLARE_NAMED_CLASS("Resource");

	Core* GetCore() { return mCore; }

	const String& GetName() const { return mName; }
};