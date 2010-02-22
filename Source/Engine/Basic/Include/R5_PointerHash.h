#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Hash meant to store pointers -- it will automatically delete them when destroyed
//============================================================================================================

template <typename Type>
class PointerHash
{
public:

	typedef Type*				TypePtr;
	typedef PointerArray<Type>	Values;
	typedef Array<uint>			Keys;
	typedef Array<uint>			Indices;

protected:

	Keys				mKeys;			// Array of unique keys that have been set
	Values				mValues;		// Matching array of unique values that have been set
	Indices				mIndices;		// 1-based index array referencing the two arrays above
	Thread::Lockable	mLock;			// Thread-safe locking functionality

public:

	PointerHash() {}
	~PointerHash() { Release(); }

	inline void				Lock()					const	{ mLock.Lock();	}
	inline void				Unlock()				const	{ mLock.Unlock();	}
	inline uint				GetNumberOfIndices()	const	{ return mIndices.GetSize(); }
	inline const Keys&		GetAllKeys()			const	{ return mKeys;	}
	inline const Values&	GetAllValues()			const	{ return mValues;	}
	inline Values&			GetAllValues()					{ return mValues;	}

	inline void Clear()
	{
		mKeys.Clear();
		mValues.Clear();
		mIndices.Clear();
	}

	inline void Release()
	{
		mKeys.Release();
		mValues.Release();
		mIndices.Release();
	}

	inline uint GetSizeInMemory() const
	{
		return	mKeys.GetSizeInMemory()	+
				mValues.GetSizeInMemory()	+
				mIndices.GetSizeInMemory();
	}

private:

	inline uint _KeyToID	(uint key)	const	{ return (mIndices.GetSize() - 1) & key;	}
	inline uint _IDToIndex	(uint id)	const	{ return mIndices[id] - 1;					}
	inline uint _KeyToIndex (uint key)	const	{ return (mIndices.IsValid() ? mIndices[_KeyToID(key)] - 1 : 0xFFFFFFFF);	}

	// Completely discards then rebuilds all indices based on the current set of keys
	void _RebuildIndices()
	{
		// Clear all current memory
		mIndices.MemsetZero();

		uint index, mask = mIndices.GetSize() - 1;

		// Run through all keys and recreate the indices
		for (uint i = 0, imax = mKeys.GetSize(); i < imax; )
		{
			index = mask & mKeys[i];
			mIndices[index] = ++i;
		}
	}

public:

	// Returns the value if the entry exists, 0 otherwise
	inline const TypePtr GetIfExists (uint key) const
	{
		uint index = _KeyToIndex(key);
		return (index < mKeys.GetSize() && mKeys[index] == key) ? mValues[index] : 0;
	}

	// Non-constant version of the function above
	inline TypePtr GetIfExists (uint key)
	{
		uint index = _KeyToIndex(key);
		return (index < mKeys.GetSize() && mKeys[index] == key) ? mValues[index] : 0;
	}

	// Checks whether the specified key exists is in the hash
	inline bool Exists (uint key) const
	{
		uint index = _KeyToIndex(key);
		return (index < mKeys.GetSize() && mKeys[index] == key);
	}

	// Removes a single entry from the list
	void Delete (uint key)
	{
		uint id		= _KeyToID(key);
		uint index	= _IDToIndex(id);

		if (index < mKeys.GetSize() && mKeys[index] == key)
		{
			mKeys.RemoveAt(index);
			mValues.DeleteAt(index);
			mIndices[id] = 0;

			// Adjust all the indices that follow this one
			for (uint i = mIndices.GetSize(); i > 0; )
			{
				if (mIndices[--i] > index)
				{
					--mIndices[i];
				}
			}
		}
	}

	// Retrieves a value from the hash -- but it will only be valid if it exists, so be careful. In most
	// cases you will either want to use an Exists() check first, or simply use the GetIfExists function.
	inline const TypePtr operator [] (uint key) const
	{
		uint index = _KeyToIndex(key);
		return mValues[index];
	}

	// Retrieves a value from the hash, inserting a new one if necessary
	TypePtr& operator [] (uint key)
	{
		// From this point on there must be a valid array to work with
		if ( mIndices.IsEmpty() )
		{
			mIndices.ExpandTo(32);
			mIndices.MemsetZero();
		}

		// Get the index for this key
		uint index = _KeyToIndex(key);

		if (index != 0xFFFFFFFF)
		{
			// If we found a valid entry, we need to match the actual key
			uint oldKey = mKeys[index];

			// If the key matches, return the value
			if (oldKey == key)
			{
				return mValues[index];
			}
			else
			{
				// If the key doesn't match, we must expand the indices until we find a set of keys that will
				const uint maxSize	= 0x1000000;
				const uint oldSize	= mIndices.GetSize();

				// Setting the key was unsuccessful due to another entry colliding with our key
				for (uint newSize = oldSize << 1; newSize < maxSize; newSize = newSize << 1)
				{
					uint mask = newSize - 1;

					// Find the next best size for the has that would make both keys unique
					if ( (mask & key) != (mask & oldKey) )
					{
						ASSERT(newSize < 65537, "Hash is getting large... intentional?");
						mIndices.Release();
						mIndices.ExpandTo(newSize);
						_RebuildIndices();
						break;
					}
				}
			}
		}

		// Append the new key to the end
		mKeys.Expand() = key;

		// Add this new entry to the index list using the current array size as the 1-based index
		mIndices[ _KeyToID(key) ] = mKeys.GetSize();

		// Return the value, but not before clearing its memory
		TypePtr& ptr = mValues.Expand();
		ptr = 0;
		return ptr;
	}

	// Sorts the hash by sorting the values and adjusting the keys to match
	void Sort()
	{
		if (mValues.IsValid())
		{
			bool sorted = false;

			uint*		startI	= mIndices.GetBuffer();
			uint*		endI	= startI + mIndices.GetSize();
			TypePtr*	start	= mValues.GetBuffer();
			TypePtr*	end		= start + mValues.GetSize();

			// Run through the arrays and sort them front-to-back
			for (TypePtr* last = end; last > start && !sorted; )
			{
				--last;
				sorted = true;

				uint*	 currI	= startI;
				TypePtr* curr	= start;
				TypePtr* next	= curr + 1;

				for (; curr < last; ++currI, ++curr, ++next )
				{
					if ( *(*next) < *(*curr) )
					{
						Swap<TypePtr>(*curr, *next);
						Swap<uint>(*currI, *(currI+1));
						sorted = false;
					}
				}
			}
		}
	}

public:

	// Convenience functions
	inline		 TypePtr	GetIfExists (const char*   key)			{ return  GetIfExists( HashKey(key) );			}
	inline		 TypePtr	GetIfExists (const String& key)			{ return  GetIfExists( HashKey(key) );			}
	inline const TypePtr	GetIfExists (const char*   key)	const	{ return  GetIfExists( HashKey(key) );			}
	inline const TypePtr	GetIfExists (const String& key)	const	{ return  GetIfExists( HashKey(key) );			}
	inline void				Delete		(const char*   key)			{		  Delete( HashKey(key) );				}
	inline void				Delete		(const String& key)			{		  Delete( HashKey(key.GetBuffer()) );	}
	inline bool				Exists		(const char*   key)	const	{ return  Exists( HashKey(key) );				}
	inline bool				Exists		(const String& key)	const	{ return  Exists( HashKey(key.GetBuffer()) );	}
	inline		 TypePtr&	operator [] (const char*   key)			{ return (*this)[ HashKey(key) ];				}
	inline		 TypePtr&	operator [] (const String& key)			{ return (*this)[ HashKey(key.GetBuffer()) ];	}
	inline const TypePtr	operator [] (const char*   key)	const	{ return (*this)[ HashKey(key) ];				}
	inline const TypePtr	operator [] (const String& key) const	{ return (*this)[ HashKey(key.GetBuffer()) ];	}
};