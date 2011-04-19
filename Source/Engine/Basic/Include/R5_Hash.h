#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Function that calculates a hash value from a string
// Author: Michael Lyashenko
//============================================================================================================

inline uint HashKey (const char* str)
{
	if (str == 0) return 0;

	uint key = 0;
	uint offset = 1357980759;

	for (uint count = 0; *str != 0; ++str, ++count)
	{
		key += (*str & 31) ^ offset;
		key += count & (offset >> 15);
		offset = key ^ (((~offset) >> 7) | (offset << 25));
	}
	return key;
}

inline uint HashKey (const String& str) { return HashKey( str.GetBuffer() ); }

//============================================================================================================
// Thread-safe function that can be used to generate ever-incrementing unique identifiers
//============================================================================================================

uint GenerateUID();

//============================================================================================================
// uint-based hash template
//============================================================================================================

template <typename Type>
class Hash
{
public:

	typedef Array<Type>		Values;
	typedef Array<uint>		Keys;
	typedef Array<uint>		Indices;

protected:

	Keys				mKeys;		// Array of unique keys that have been set
	Values				mValues;	// Matching array of unique values that have been set
	Indices				mIndices;	// 1-based index array referencing the two arrays above
	Thread::Lockable	mLock;		// Thread-safe locking functionality

public:

	Hash() {}
	~Hash() { Release(); }

	inline void				Lock()					const	{ mLock.Lock();	}
	inline void				Unlock()				const	{ mLock.Unlock();	}
	inline bool				IsValid()				const	{ return mKeys.IsValid(); }
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

	// Returns a pointer to the value of a key if it exists, 0 otherwise
	inline const Type* GetIfExists (uint key) const
	{
		uint index = _KeyToIndex(key);
		return (index < mKeys.GetSize() && mKeys[index] == key) ? &mValues[index] : 0;
	}

	// Non-constant version of the function above
	inline Type* GetIfExists (uint key)
	{
		uint index = _KeyToIndex(key);
		return (index < mKeys.GetSize() && mKeys[index] == key) ? &mValues[index] : 0;
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
			mValues.RemoveAt(index);
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
	inline const Type& operator [] (uint key) const
	{
		uint index = _KeyToIndex(key);
		return mValues[index];
	}

	// Retrieves a value from the hash, inserting a new one if necessary
	Type& operator [] (uint key)
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
						ASSERT(newSize < 262145, "Hash is getting large... intentional?");
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

		// Return the value
		return mValues.Expand();
	}

	// Convenience functionality
	inline const Type&	operator [] (int key) const { return (*this)[(uint)key]; }
	inline Type&		operator [] (int key)		{ return (*this)[(uint)key]; }

public:

	// Convenience functions
	inline		 Type*	GetIfExists (const char*   key)			{ return  GetIfExists( HashKey(key) );			}
	inline		 Type*	GetIfExists (const String& key)			{ return  GetIfExists( HashKey(key) );			}
	inline const Type*	GetIfExists (const char*   key)	const	{ return  GetIfExists( HashKey(key) );			}
	inline const Type*	GetIfExists (const String& key)	const	{ return  GetIfExists( HashKey(key) );			}
	inline void			Delete		(const char*   key)			{		  Delete( HashKey(key) );				}
	inline void			Delete		(const String& key)			{		  Delete( HashKey(key.GetBuffer()) );	}
	inline bool			Exists		(const char*   key)	const	{ return  Exists( HashKey(key) );				}
	inline bool			Exists		(const String& key)	const	{ return  Exists( HashKey(key.GetBuffer()) );	}
	inline		 Type&	operator [] (const char*   key)			{ return (*this)[ HashKey(key) ];				}
	inline		 Type&	operator [] (const String& key)			{ return (*this)[ HashKey(key.GetBuffer()) ];	}
	inline const Type&	operator [] (const char*   key)	const	{ return (*this)[ HashKey(key) ];				}
	inline const Type&	operator [] (const String& key) const	{ return (*this)[ HashKey(key.GetBuffer()) ];	}
};