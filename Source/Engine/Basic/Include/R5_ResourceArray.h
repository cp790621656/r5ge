#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Pointer array for named resources -- used by manager classes
// Author: Michael Lyashenko
//============================================================================================================

template <typename Type>
class ResourceArray : public PointerArray<Type>
{
public:

	typedef PointerArray<Type> BASE;

	ResourceArray()				: PointerArray<Type>() {}
	ResourceArray(uint reserve) : PointerArray<Type>(reserve) {}

private:

	// I don't want to allow copying of this class
	ResourceArray(const ResourceArray& in) {}
	void operator =  (const ResourceArray& in) {}

public:

	// Adds a new entry to the list, giving it the specified name
	Type* Add (const String& name, bool threadSafe = true)
	{
		if (threadSafe) BASE::Lock();
		Type* ptr = (BASE::Expand() = new Type(name));
		if (threadSafe) BASE::Unlock();
		return ptr;
	}

	// Adds a unique entry to the array, or returns an existing one
	Type* AddUnique (const String& name, bool threadSafe = true)
	{
		if (threadSafe) BASE::Lock();
		for ( Type **ptr = BASE::mArray, **end = BASE::mArray + BASE::mSize; ptr != end; ++ptr )
			if ((*ptr)->GetName() == name)
			{
				if (threadSafe) BASE::Unlock();
				return *ptr;
			}

		Type* out = (BASE::Expand() = new Type(name));
		if (threadSafe) BASE::Unlock();
		return out;
	}

	// Returns an index of the entry with the specified name
	Type* Find (const String& name, bool threadSafe = true)
	{
		Type* ret (0);
		if (threadSafe) BASE::Lock();
		for ( Type **ptr = BASE::mArray, **end = BASE::mArray + BASE::mSize; ptr != end; ++ptr )
		{
			if ((*ptr)->GetName() == name)
			{
				ret = *ptr;
				break;
			}
		}
		if (threadSafe) BASE::Unlock();
		return ret;
	}

	Type** GetStart() { return BASE::mArray; }
	Type** GetEnd()   { return BASE::mArray + BASE::mSize; }
};