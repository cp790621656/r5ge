#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Pointer array for named resources -- used by manager classes
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
	Type* Add (const String& name)
	{
		BASE::Lock();
		Type* ptr = (BASE::Expand() = new Type(name));
		BASE::Unlock();
		return ptr;
	}

	// Adds a unique entry to the array, or returns an existing one
	Type* AddUnique (const String& name)
	{
		BASE::Lock();
		for ( Type **ptr = BASE::mArray, **end = BASE::mArray + BASE::mSize; ptr != end; ++ptr )
			if ((*ptr)->GetName() == name)
			{
				BASE::Unlock();
				return *ptr;
			}

		Type* out = (BASE::Expand() = new Type(name));
		BASE::Unlock();
		return out;
	}

	// Returns an index of the entry with the specified name
	Type* Find (const String& name)
	{
		Type* ret (0);
		BASE::Lock();
		for ( Type **ptr = BASE::mArray, **end = BASE::mArray + BASE::mSize; ptr != end; ++ptr )
		{
			if ((*ptr)->GetName() == name)
			{
				ret = *ptr;
				break;
			}
		}
		BASE::Unlock();
		return ret;
	}

	Type** GetStart() { return BASE::mArray; }
	Type** GetEnd()   { return BASE::mArray + BASE::mSize; }

	// Deletes the specified entry from the array
	bool DeleteByName (const String& name)
	{
		BASE::Lock();
		for ( Type **start = BASE::mArray, **end = BASE::mArray + BASE::mSize; start != end; ++start )
		{
			if ((*start) != 0 && (*start)->GetName() == name)
			{
				delete *start;
				--BASE::mSize;
				--end;

				if (start < end)
					memmove(start, start + 1, end - start);

				BASE::Unlock();
				return true;
			}
		}
		BASE::Unlock();
		return false;
	}
};